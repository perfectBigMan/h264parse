#ifndef FRAMEOTHER_H
#define FRAMEOTHER_H

#include "Frame.h"

class FrameOther : public Frame
{
public:
    FrameOther(uint8* file, int seq, int offset, int len);

    int parse();
};

#endif // FRAMEOTHER_H
