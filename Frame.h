#ifndef FRAME_H
#define FRAME_H

#include <QString>
#include <QColor>
#include "def.h"
#include "ExpGolomb.h"

enum NAL_TYPE {
    NAL_TYPE_UNKNOWN, // 0
    NAL_SLICE, // 1
    NAL_SLICE_DPA,
    NAL_SLICE_DPB,
    NAL_SLICE_DPC,
    NAL_SLICE_IDR, //5
    NAL_SLICE_SEI, //6
    NAL_SPS, // 7
    NAL_PPS, // 8
    NAL_AUD, // 9
    NAL_FILLER = 12
};


class Frame {
public:
    Frame(uint8* file, int seq, int offset, int len);
    virtual ~Frame();
    int getNalType();
    int getOffset() { return offset; }
    int getLen() { return len; }
    int getSeq() { return seq; }
    uint8* getData() { return file + offset; }
    uint8* getYuvData() { return yuv; }
    int getYuvLen() { return yuvlen; }
    void releaseYuvData();
    QString getInfo() { return info; }
    QString getStartCode() { return startCode; }
    QString getTypedesc() { return typedesc; }
    Qt::GlobalColor getColor() { return color; }

    void setYuvFrame(uint8* buf, int len);

    virtual int parse() { return 0; }

    static int nalType(uint8* buf);
    static int width;
    static int height;
    static int frameCropLeftOffset;
    static int frameCropRightOffset;
    static int frameCropTopOffset;
    static int frameCropBottomOffset;
    static int profileIdc;
    static int levelIdc;
protected:
    int seq;
    int offset;
    int len;
    int naltype;
    uint8* file;
    QString startCode;
    QString typedesc;
    QString info;
    Qt::GlobalColor color;
    bool parsed = false;
    uint8* yuv = NULL;
    int yuvlen = 0;



};

#endif // FRAME_H
