#include "decoder.h"


typedef struct {
    AVCodec* codec;
    AVCodecParserContext* parser;
    AVCodecContext* codec_ctx;
    AVFrame* frame;
    AVPacket* pkt;
    uint8_t* yuv_buffer;
    int yuv_buffer_size;

//    video_callback cb;
} context;

static int copyYuvData(AVFrame *frame, uint8_t *buffer, int width, int height) {
    int ret		    = 0;
    uint8_t *src	= NULL;
    uint8_t *dst	= buffer;
    int i = 0;
    do {
        if (frame == NULL || buffer == NULL) {
            ret = -1;
            break;
        }
        if (!frame->data[0] || !frame->data[1] || !frame->data[2]) {
            ret = -1;
            break;
        }
        for (i = 0; i < height; i++) {
            src = frame->data[0] + i * frame->linesize[0];
            memcpy(dst, src, width);
            dst += width;
        }
        for (i = 0; i < height / 2; i++) {
            src = frame->data[1] + i * frame->linesize[1];
            memcpy(dst, src, width / 2);
            dst += width / 2;
        }
        for (i = 0; i < height / 2; i++) {
            src = frame->data[2] + i * frame->linesize[2];
            memcpy(dst, src, width / 2);
            dst += width / 2;
        }
    } while (0);
    return ret;	
}

static int decode_pkt(context* handle, AVPacket *pkt, video_callback cb, void* arg)
{
    int ret;
    AVCodecContext *dec_ctx = handle->codec_ctx;
    AVFrame *frame = handle->frame;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        printf("Error sending a packet for decoding.\n");
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            printf( "Error during decoding.\n");
            return -1;
        }
        printf("Saving frame %d.\n", dec_ctx->frame_number);
        if(!handle->yuv_buffer) {
            handle->yuv_buffer_size = avpicture_get_size(AV_PIX_FMT_YUV420P, frame->width, frame->height);
            handle->yuv_buffer = (uint8_t *)av_mallocz(handle->yuv_buffer_size);
        }

        /* the picture is allocated by the decoder. no need to free it */
        printf( "P5:\n\t(%d %d) %d %d %d\n", frame->width, frame->height, frame->linesize[0], frame->linesize[1], frame->linesize[2]);
        copyYuvData(frame, handle->yuv_buffer, frame->width, frame->height);
        if(cb) {
            cb(handle->yuv_buffer, handle->yuv_buffer_size, arg);
        }
    }
    
    return 0;
}

int parse_to_frame(void* _handle, uint8_t *buffer, int buffer_size, uint8_t** out, int* out_size) {
    context* handle = (context*)_handle;
    int ret = av_parser_parse2(handle->parser, handle->codec_ctx, out, out_size, buffer,
                             buffer_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    return ret;                             
}

int decode(void* _handle, uint8_t *buffer, int buffer_size, video_callback cb, void* arg) {
    context* handle = (context*)_handle;
    if(!handle || !handle->pkt) return -1;
    int ret = 0;
    if(!buffer || buffer_size <= 0) {
        ret = decode_pkt(handle, NULL, cb, arg);
    } else {
        handle->pkt->data = buffer;
        handle->pkt->size = buffer_size;
        ret = decode_pkt(handle, handle->pkt, cb, arg);
    }
    return ret;
}

void close_decoder(void* _handle) {
    context* handle = (context*)_handle;
    if(handle == NULL) {
        return;
    }

    if(handle->parser) {
        av_parser_close(handle->parser);
        handle->parser = NULL;
    }
    if(handle->codec_ctx) {
        avcodec_free_context(&handle->codec_ctx);
        handle->codec_ctx = NULL;
    }

    if(handle->yuv_buffer) {
        av_freep(&handle->yuv_buffer);
        handle->yuv_buffer_size = 0;
    }

    if(handle->frame) {
        av_frame_free(&handle->frame);
        handle->frame = NULL;
    }
    if(handle->pkt) {
        av_packet_free(&handle->pkt);
        handle->pkt = NULL;
    }
    free(handle);
}

void* open_decoder() {
    context* handle = (context*)malloc(sizeof(context));
    if(!handle) {
        return 0;
    }
    do {
        memset(handle, 0, sizeof(context));
        av_register_all();
        handle->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if(!handle->codec) {
            printf("Codec not found.\n");
            break;
        }
        
        handle->parser = av_parser_init(handle->codec->id);
        if(!handle->parser) {
            printf( "parser not found.\n");
            break;
        }

        handle->codec_ctx = avcodec_alloc_context3(handle->codec);
        if(!handle->codec_ctx) {
            printf( "Could not allocate video codec context.\n");
            break;
        }

        if(avcodec_open2(handle->codec_ctx, handle->codec, NULL) < 0) {
            printf( "Could not open codec.\n");
            break;
        }

        handle->frame = av_frame_alloc();
        if(!handle->frame) {
            printf("Could not allocate video frame.\n");
            break;
        }

        handle->pkt = av_packet_alloc();
        if(!handle->pkt) {
            printf( "Packet alloc error.\n");
            break;
        }
        return (void*)handle;
    } while(0);
    close_decoder((void*)handle);
    return 0;
}
