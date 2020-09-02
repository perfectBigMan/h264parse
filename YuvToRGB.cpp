#include "YuvToRGB.h"

YuvToRGB::YuvToRGB()
{

}

YuvToRGB::~YuvToRGB() {
    if (img_convert_ctx) {
        sws_freeContext(img_convert_ctx);
        img_convert_ctx = NULL;
    }
}

int YuvToRGB::init(int width, int height) {
    this->width = width;
    this->height = height;


    //width和heigt为传入的分辨率的大小，实际应用我传的1280*720
    int yuvSize = width * height * 3 / 2;
    yuvBuffer = (uint8_t *)malloc(yuvSize);
    //为每帧图像分配内存
    pFrame = av_frame_alloc();

    pFrameRGB = av_frame_alloc();
    rgbLen = avpicture_get_size(AV_PIX_FMT_RGBA, width,height);
    rgbBuffer = (uint8_t *) av_malloc(rgbLen * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, rgbBuffer, AV_PIX_FMT_RGBA, width, height);

    //特别注意 img_convert_ctx 该在做H264流媒体解码时候，发现sws_getContext /sws_scale内存泄露问题，
    //注意sws_getContext只能调用一次，在初始化时候调用即可，另外调用完后，在析构函数中使用sws_free_Context，将它的内存释放。
    //设置图像转换上下文
    img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height,
                                  AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);

}

void YuvToRGB::convert(uint8_t* buf, int len) {
    avpicture_fill((AVPicture *) pFrame, buf, AV_PIX_FMT_YUV420P, width, height);//这里的长度和高度跟之前保持一致
    //转换图像格式，将解压出来的YUV420P的图像转换为RGB的图像
    sws_scale(img_convert_ctx,
            (uint8_t const * const *) pFrame->data,
            pFrame->linesize, 0, height, pFrameRGB->data,
            pFrameRGB->linesize);

    FILE* fp = fopen("out.rgb", "wb");
    if (fp) {
        printf("%s:%d||%d\n", __FUNCTION__, __LINE__, rgbLen);
        fwrite(rgbBuffer, 1, rgbLen, fp);
        fclose(fp);
    }
}
