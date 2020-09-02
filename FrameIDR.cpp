#include "FrameIDR.h"

FrameIDR::FrameIDR(uint8* file, int seq, int offset, int len): Frame(file, seq, offset, len)
{
     color = Qt::red;
}

int FrameIDR::parse() { return 0;}
