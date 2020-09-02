#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <QString>
#include <vector>
#include "Frame.h"
#include "decoder.h"



class MediaFile
{
public:
    MediaFile(QString name);
    ~MediaFile();
    int parse();
    std::vector<Frame*>* getFrameList() { return &mFrameList; }
    int frameSize();
    Frame* getFrame(int id);
    unsigned char getByte(int pos);
    int parseFrameToYuv(Frame* f);
private:
    int readFile();
    int parseToFrame();
    int parseAllFrameToYuv();
    void clearFrameList();
private:
    QString mName;
    unsigned char* mFileContent;
    unsigned int mFileContentLen;
    std::vector<Frame*> mFrameList;
    int mWidth;
    int mHeight;
    void* handle = NULL;
};

#endif // MEDIAFILE_H
