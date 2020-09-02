#include "FrameSPS.h"

FrameSPS::FrameSPS(uint8* file, int seq, int offset, int len) : Frame(file, seq, offset, len)
{
    color = Qt::green;
    if (Frame::width == 0 || Frame::height == 0) {
        parse();
        Frame::profileIdc = profileIdc;
        Frame::levelIdc = levelIdc;
        Frame::width = (((picWidthInMbsMinus1 + 1) * 16) - frameCropLeftOffset * 2 - frameCropRightOffset * 2) * sarScale;
        Frame::height = ((2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16) - ((frameMbsOnlyFlag ? 2 : 4) * (frameCropTopOffset + frameCropBottomOffset)),
        Frame::frameCropLeftOffset = frameCropLeftOffset;
        Frame::frameCropRightOffset = frameCropRightOffset;
        Frame::frameCropTopOffset = frameCropTopOffset;
        Frame::frameCropBottomOffset = frameCropBottomOffset;
        printf("width: %d height: %d\n", Frame::width, Frame::height);
    }
}

int FrameSPS::parse() {
    if (parsed == true) return 0;

    printf("FrameSPS parse\n");
    uint8* buf = file + offset + 4;

    ExpGolomb* decoder = new ExpGolomb(buf, len);
    if (decoder) {
        uint8 nalhead = decoder->readUByte();
        profileIdc = decoder->readUByte();
        uint8 profileCompat = decoder->readBits(5); // constraint_set[0-4]_flag, u(5)
        decoder->skipBits(3);
        levelIdc = decoder->readUByte();
        decoder->skipUEG();
        if (profileIdc == 100 ||
            profileIdc == 110 ||
            profileIdc == 122 ||
            profileIdc == 244 ||
            profileIdc == 44 ||
            profileIdc == 83 ||
            profileIdc == 86 ||
            profileIdc == 118 ||
            profileIdc == 128) {
            int chromaFormatIdc = decoder->readUEG();
            if (chromaFormatIdc == 3) {
                decoder->skipBits(1); // separate_colour_plane_flag
            }
            decoder->skipUEG(); // bit_depth_luma_minus8
            decoder->skipUEG(); // bit_depth_chroma_minus8
            decoder->skipBits(1); // qpprime_y_zero_transform_bypass_flag
            if (decoder->readBoolean()) { // seq_scaling_matrix_present_flag
                int scalingListCount = (chromaFormatIdc != 3) ? 8 : 12;
                for (int i = 0; i < scalingListCount; ++i) {
                    if (decoder->readBoolean()) { // seq_scaling_list_present_flag[ i ]
                        if (i < 6) {
                            skipScalingList(decoder, 16);
                        } else {
                            skipScalingList(decoder, 64);
                        }
                    }
                }
            }
        }
        decoder->skipUEG(); // log2_max_frame_num_minus4
        int picOrderCntType = decoder->readUEG();
        printf("picOrderCntType %d\n", picOrderCntType);
        if (picOrderCntType == 0) {
            decoder->readUEG(); // log2_max_pic_order_cnt_lsb_minus4
        } else if (picOrderCntType == 1) {
            decoder->skipBits(1); // delta_pic_order_always_zero_flag
            decoder->skipEG(); // offset_for_non_ref_pic
            decoder->skipEG(); // offset_for_top_to_bottom_field
            int numRefFramesInPicOrderCntCycle = decoder->readUEG();
            for (int i = 0; i < numRefFramesInPicOrderCntCycle; ++i) {
                decoder->skipEG(); // offset_for_ref_frame[ i ]
            }
        }
        decoder->skipUEG(); // max_num_ref_frames
        decoder->skipBits(1); // gaps_in_frame_num_value_allowed_flag
        picWidthInMbsMinus1 = decoder->readUEG();
        picHeightInMapUnitsMinus1 = decoder->readUEG();
        printf("picWidthInMbsMinus1: %d\n", picWidthInMbsMinus1);
        printf("picHeightInMapUnitsMinus1: %d\n", picHeightInMapUnitsMinus1);

        frameMbsOnlyFlag = decoder->readBits(1);
        printf("frameMbsOnlyFlag: %d\n", frameMbsOnlyFlag);
        if (frameMbsOnlyFlag == 0) {
            decoder->skipBits(1); // mb_adaptive_frame_field_flag
        }
        decoder->skipBits(1); // direct_8x8_inference_flag
        frame_cropping_flag = decoder->readBoolean();
        printf("frame_cropping_flag: %d\n", frame_cropping_flag);
        if (frame_cropping_flag) { // frame_cropping_flag
            frameCropLeftOffset = decoder->readUEG();
            frameCropRightOffset = decoder->readUEG();
            frameCropTopOffset = decoder->readUEG();
            frameCropBottomOffset = decoder->readUEG();
            printf("frameCropLeftOffset: %d\n", frameCropLeftOffset);
            printf("frameCropRightOffset: %d\n", frameCropRightOffset);
            printf("frameCropTopOffset: %d\n", frameCropTopOffset);
            printf("frameCropBottomOffset: %d\n", frameCropBottomOffset);
        }
        vui_parameters_present_flag = decoder->readBoolean();
        if (vui_parameters_present_flag) {
            aspect_ratio_info_present_flag = decoder->readBoolean();
            if (aspect_ratio_info_present_flag) {
                // aspect_ratio_info_present_flag
                int aspectRatioIdc = decoder->readUByte();
                printf("aspectRatioIdc %d\n", aspectRatioIdc);
                switch (aspectRatioIdc) {
                    case 1: sarScale = 1/1; break;
                    case 2: sarScale = 12.0/11; break;
                    case 3: sarScale = 10.0/11; break;
                    case 4: sarScale = 16.0/11; break;
                    case 5: sarScale = 40.0/33; break;
                    case 6: sarScale = 24.0/11; break;
                    case 7: sarScale = 20.0/11; break;
                    case 8: sarScale = 32.0/11; break;
                    case 9: sarScale = 80.0/33; break;
                    case 10: sarScale = 18.0/11; break;
                    case 11: sarScale = 15.0/11; break;
                    case 12: sarScale = 64.0/33; break;
                    case 13: sarScale = 160.0/99; break;
                    case 14: sarScale = 4.0/3; break;
                    case 15: sarScale = 3.0/2; break;
                    case 16: sarScale = 2.0/1; break;
                    case 255: {
                        sarScale = (decoder->readUByte() << 8 | decoder->readUByte()) * 1.0 / (decoder->readUByte() << 8 | decoder->readUByte());
                        break;
                    }
                }
            }
            if (decoder->readBoolean()) { decoder->skipBits(1); }

            if (decoder->readBoolean()) {
                decoder->skipBits(4);
                if (decoder->readBoolean()) {
                    decoder->skipBits(24);
                }
            }
            if (decoder->readBoolean()) {
                decoder->skipUEG();
                decoder->skipUEG();
            }
            if (decoder->readBoolean()) {
                int unitsInTick = decoder->readUInt();
                int timeScale = decoder->readUInt();
                int fixedFrameRate = decoder->readBoolean();
                int frameDuration = timeScale / (2 * unitsInTick);
            }
        }

//        int width = (((picWidthInMbsMinus1 + 1) * 16) - frameCropLeftOffset * 2 - frameCropRightOffset * 2) * sarScale;
//        int height = ((2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16) - ((frameMbsOnlyFlag ? 2 : 4) * (frameCropTopOffset + frameCropBottomOffset));


        printf("nalhead %x\n", nalhead);
        printf("profileIdc %d\n", profileIdc);
        printf("levelIdc %d\n", levelIdc); // level_idc u(8)


        delete decoder;
        parsed = true;
    }
    return 0;
}


void FrameSPS::skipScalingList(ExpGolomb* decoder, int count) {
    int lastScale = 8;
    int nextScale = 8;
    int deltaScale;
    for (int j = 0; j < count; j++) {
        if (nextScale != 0) {
            deltaScale = decoder->readEG();
            nextScale = (lastScale + deltaScale + 256) % 256;
        }
        lastScale = (nextScale == 0) ? lastScale : nextScale;
    }
}
