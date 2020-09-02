#include "MediaFile.h"
#include <QFile>
#include <QDebug>
#include "FrameSPS.h"
#include "FrameIDR.h"
#include "FrameOther.h"


void handle_video(uint8_t* buf, int size, void* arg) {
    printf("handle_video %d\n", size);

    Frame* f = (Frame*)arg;
    f->setYuvFrame(buf, size);
}

MediaFile::MediaFile(QString name)
{
    mName = name;
    handle = open_decoder();
}

MediaFile::~MediaFile() {
    if (handle) {
        close_decoder(handle);
        handle = NULL;
    }
    if (mFileContent) {
        delete [] mFileContent;
        mFileContent = NULL;
    }
    clearFrameList();
    Frame::width = 0;
    Frame::height = 0;
}

int MediaFile::parse() {
    if (readFile() == -1) return -1;
    if (parseToFrame() == -1) return -1;

//    parseFrameToYuv();
    return 0;
}

int MediaFile::frameSize() {
    return mFrameList.size();
}

Frame* MediaFile::getFrame(int id) {
    return mFrameList[id];
}

unsigned char MediaFile::getByte(int pos) {
    return mFileContent[pos];
}

int MediaFile::parseFrameToYuv(Frame* f) {
    int seq = f->getSeq();
    int naltype = f->getNalType();
    if (naltype != NAL_SLICE_IDR && naltype != NAL_SLICE) {
        return -1;
    }

    int startseq = 0;
    if (naltype == NAL_SLICE_IDR) {
        while(--seq >= 0) {
            Frame* tmp = mFrameList[seq];
            if (tmp->getNalType() == NAL_SLICE) {
                startseq = seq + 1;
                break;
            }
        }
    } else if (naltype == NAL_SLICE) {
        while(--seq >= 0) {
            if (mFrameList[seq]->getNalType() == NAL_SLICE_IDR) {
                break;
            }
        }
        while(--seq >= 0) {
            if (mFrameList[seq]->getNalType() == NAL_SLICE) {
                startseq = seq + 1;
                break;
            }
        }
    }

    printf("start seq %d now %d\n", startseq, f->getSeq());
    int start = -1;
    for(int i = startseq; i <= f->getSeq(); i++) {
        Frame* curf = mFrameList[i];
        int naltype = curf->getNalType();
        if (naltype == NAL_SLICE_IDR || naltype == NAL_SLICE) { //i
            int buflen;
            uint8* buf;
            if (start != -1) {
                buf = mFileContent + start;
                buflen = curf->getOffset() - start + curf->getLen();
                // printf("%s:%d||%d %d %d\n", __FUNCTION__, __LINE__, id, start, len);
            } else {
                buf = mFileContent + curf->getOffset();
                buflen = curf->getLen();
                // printf("%s:%d||%d %d %d\n", __FUNCTION__, __LINE__, id, f->getOffset(), f->getLen());
            }
            if (handle) {
                if (i == f->getSeq()) {
                    decode(handle, buf, buflen, handle_video, curf);
                } else {
                    decode(handle, buf, buflen, NULL, NULL); 
                }
            }
            start = -1;
        } else {
            if (start == -1) {
                start = curf->getOffset();
            }
        }
    }
    return 0;

}

int MediaFile::readFile() {
    QFile file(mName);
    bool ret = file.open(QIODevice::ReadOnly);
    if (!ret) {
        return -1;
    }

    mFileContentLen = file.size();
    mFileContent = new unsigned char[mFileContentLen]();
    qDebug() << "file size:" << mFileContentLen;

    char buffer[1024] = {0};
    unsigned int bufferpos = 0;
    while(!file.atEnd()) {
        memset(buffer, 0, sizeof(buffer));
        int readlen = file.read(buffer, sizeof(buffer));
        memcpy(mFileContent + bufferpos, buffer, readlen);
        bufferpos += readlen;
    } // end of while

//    qDebug() << "bufferpos:" << bufferpos;
    file.close();
    return 0;
}

int MediaFile::parseToFrame() {
    char value;
    int i = 0;
    int state = 0;
    int length = mFileContentLen;
    int lastIndex = 0;
    int startcodenum = 4;
    while(i < length) {
        value = mFileContent[i];
        switch (state) {
            case 0:
                if (value == 0) {
                    state = 1;
                }
                break;
            case 1:
                if (value == 0) {
                    state = 2;
                } else {
                    state = 0;
                }
                break;
            case 2:
            case 3:
                if (value == 0) {
                    state = 3;
                } else if (value == 1 && i < length) {
                    if (lastIndex) {
                        if (startcodenum == 0) {
                            startcodenum = state + 1;
                        }
                        int startpos = lastIndex - state;
                        int overpos = i - state - 1;
                        int len = overpos - startpos + 1;
                        //qDebug() << QString("%1 %2 ~ %3 len:%5 ").arg(startpos, 8, 16).arg(startpos).arg(overpos).arg(len);
                        Frame* frame;
                        if (Frame::nalType(mFileContent + startpos) == NAL_SPS) {
                            frame = new FrameSPS(mFileContent, mFrameList.size(), startpos, len);
                        } else if (Frame::nalType(mFileContent + startpos) == NAL_SLICE_IDR) {
                            frame = new FrameIDR(mFileContent, mFrameList.size(), startpos, len);
                        } else {
                            frame = new FrameOther(mFileContent, mFrameList.size(), startpos, len);
                        }

                        mFrameList.push_back(frame);
                    }
                    lastIndex = i;
                    state = 0;
                } else {
                    state = 0;
                }
                break;
            default:
                break;
        } // end of switch

        i++;
    } // end of while
    if (lastIndex) {
        int startpos = lastIndex - startcodenum + 1;
        int overpos = length - 1;
        int len = overpos - startpos + 1;
        //qDebug() << QString("%1 %2 ~ %3 len:%5 ").arg(startpos, 8, 16).arg(startpos).arg(overpos).arg(len);
        Frame* frame;
        if (Frame::nalType(mFileContent + startpos) == NAL_SPS) {
            frame = new FrameSPS(mFileContent, mFrameList.size(), startpos, len);
        } else if (Frame::nalType(mFileContent + startpos) == NAL_SLICE_IDR) {
            frame = new FrameIDR(mFileContent, mFrameList.size(), startpos, len);
        } else {
            frame = new FrameOther(mFileContent, mFrameList.size(), startpos, len);
        }

        mFrameList.push_back(frame);
    }

    return 0;
}

int MediaFile::parseAllFrameToYuv() {
    std::vector<Frame*>::iterator it = mFrameList.begin();
    int start = -1;

    int id = 0;
    while(it != mFrameList.end()) {
        Frame* f = *it;
        int naltype = f->getNalType();

        if (naltype == NAL_SLICE_IDR || naltype == NAL_SLICE) { //i
            int buflen;
            uint8* buf;
            if (start != -1) {
                buf = mFileContent + start;
                buflen = f->getOffset() - start + f->getLen();
                // printf("%s:%d||%d %d %d\n", __FUNCTION__, __LINE__, id, start, len);
            } else {
                buf = mFileContent + f->getOffset();
                buflen = f->getLen();
                // printf("%s:%d||%d %d %d\n", __FUNCTION__, __LINE__, id, f->getOffset(), f->getLen());
            }
            if (handle) {
                decode(handle, buf, buflen, handle_video, f);
            }
            start = -1;
            id++;
        } else {
            if (start == -1) {
                start = f->getOffset();
            }
        }
        it++;
    }
    return 0;
}




void MediaFile::clearFrameList() {
    std::vector<Frame*>::iterator it = mFrameList.begin();
    while(it != mFrameList.end()) {
        Frame* f = *it;
        if (f) {
            delete f;
        }
        it = mFrameList.erase(it);
    }
}
