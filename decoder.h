#ifndef _DECODER_H_
#define _DECODER_H_

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/log.h>
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef void(*video_callback)(uint8_t* buf, int size, void* arg);

void* open_decoder();
void close_decoder(void* _handle);
int parse_to_frame(void* _handle, uint8_t *buffer, int buffer_size, uint8_t** out, int* out_size);
int decode(void* _handle, uint8_t *buffer, int buffer_size, video_callback cb, void* arg);

#endif //_DECODER_H_
