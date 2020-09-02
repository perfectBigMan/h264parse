#ifndef FRAMESPS_H
#define FRAMESPS_H
#include "Frame.h"
#include "ExpGolomb.h"

class FrameSPS : public Frame
{
public:
    FrameSPS(uint8* file, int seq, int offset, int len);
    int parse();
private:
    void skipScalingList(ExpGolomb* decoder, int count);
private:
    int profileIdc = 0;
    int levelIdc = 0;
    int picWidthInMbsMinus1 = 0;
    int picHeightInMapUnitsMinus1 = 0;
    int frameMbsOnlyFlag = 0;
    int frame_cropping_flag = 0;
    int frameCropLeftOffset = 0;
    int frameCropRightOffset = 0;
    int frameCropTopOffset = 0;
    int frameCropBottomOffset = 0;
    double sarScale = 1;
    int vui_parameters_present_flag = 0;
    int aspect_ratio_info_present_flag = 0;
};

#endif // FRAMESPS_H
