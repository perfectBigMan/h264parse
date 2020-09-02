#include "Frame.h"



int Frame::width = 0;
int Frame::height = 0;
int Frame::profileIdc = 0;
int Frame::levelIdc = 0;
int Frame::frameCropLeftOffset = 0;
int Frame::frameCropRightOffset = 0;
int Frame::frameCropTopOffset = 0;
int Frame::frameCropBottomOffset = 0;

struct NalDesc {
    int naltype;
    char typedesc[64];
    char info[24];
} naltype_table[] = {
    {0, "", ""},
    {1, "Coded slice of a non-IDR picture", "P Slice"},
    {2, "", ""},
    {3, "", ""},
    {4, "", ""},
    {NAL_SLICE_IDR, "Code slice of an IDR picture", "IDR"},
    {6, "", ""},
    {7, "Sequence parameter set", "SPS"},
    {8, "Picture parameter set", "PPS"}
};


Frame::Frame(uint8* file, int seq, int offset, int len) {
   this->seq = seq;
   this->offset = offset;
   this->len = len;
   this->file = file;

   if (file[offset + 3] == 1) {
       startCode = QString("00000001%1").arg(file[offset + 4], 2, 16);
       naltype = file[offset + 4] & 0x1f;
   } else if (file[offset + 2] == 1) {
       startCode = QString("000001%1").arg(file[offset + 3], 2, 16);
       naltype = file[offset + 3] & 0x1f;
   }
   if (naltype == NAL_SPS) {
   } else if (naltype == NAL_PPS) {
       color = Qt::darkYellow;
   } else if (naltype == NAL_SLICE_IDR) {
   } else {
       color = Qt::white;
   }

   NalDesc nal = naltype_table[naltype];
   typedesc = nal.typedesc;
   info = nal.info;

}

Frame::~Frame() {
    releaseYuvData();
}

int Frame::getNalType(){
    return naltype;
}


int Frame::nalType(uint8* buf) {
    int naltype;
    if (buf[3] == 1) {
        naltype = buf[4] & 0x1f;
    } else if (buf[2] == 1) {
        naltype = buf[3] & 0x1f;
    }
    return naltype;
}

void Frame::setYuvFrame(uint8* buf, int len) {
    if (yuv == NULL) {
        yuvlen = len;
        yuv = (uint8*)malloc(yuvlen);
        memcpy(yuv, buf, yuvlen);
    }
}

void Frame::releaseYuvData() {
    if (yuv) {
        free(yuv);
        yuvlen = 0;
        yuv = NULL;
    }
}