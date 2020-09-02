#ifndef FRAMEIDR_H
#define FRAMEIDR_H

#include "Frame.h"

class FrameIDR : public Frame
{
public:
    FrameIDR(uint8* file, int seq, int offset, int len);

    int parse();
};

#endif // FRAMEIDR_H
