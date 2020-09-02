#include "FrameOther.h"


FrameOther::FrameOther(uint8* file, int seq, int offset, int len): Frame(file, seq, offset, len)
{

}

int FrameOther::parse() { return 0;}
