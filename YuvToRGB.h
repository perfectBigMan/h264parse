#ifndef YUVTORGB_H
#define YUVTORGB_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/log.h>
}


class YuvToRGB
{
public:
    YuvToRGB();
    ~YuvToRGB();
    int init(int width, int height);
    void convert(uint8_t* buf, int len);
    uint8_t* getRgbBuffer() { return rgbBuffer; }
    int getRgbLen() { return rgbLen; }
private:
    int width = 0;
    int height = 0;
    uint8_t* yuvBuffer= NULL;
    AVFrame* pFrame = NULL;
    AVFrame* pFrameRGB = NULL;
    int rgbLen = 0;
    uint8_t* rgbBuffer = NULL;
    SwsContext* img_convert_ctx = NULL;
};

#endif // YUVTORGB_H
