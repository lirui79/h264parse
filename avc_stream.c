#include "avc_stream.h"


/**
 scaling_list函数实现
 [h264协议文档位置]：7.3.2.1.1 Scaling list syntax
 */
void scaling_list(bs_t *b, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag) {
    int deltaScale;
    int lastScale = 8;
    int nextScale = 8;
    
    for (int j = 0; j < sizeOfScalingList; j++) {
        
        if (nextScale != 0) {
            deltaScale = bs_read_se(b);
            nextScale = (lastScale + deltaScale + 256) % 256;
            *useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
        }
        
        scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[j];
    }
}


/**
 读取一个nalu
 @see 7.3.1 NAL unit syntax
 @see 7.4.1 NAL unit semantics
 */
unsigned int parse_nalu(nalu_t *nalu, sps_t *sps, pps_t *pps, slice_t *slice) {
    uint8_t* buffer = NULL;
    unsigned int bufSize = nalu->len;

    buffer = malloc(bufSize);
    // 1.去除nalu中的emulation_prevention_three_byte：0x03
    emulation_prevention_three_byte(nalu->buf, nalu->len, buffer, &bufSize);
    nalu->buf = buffer;
    nalu->len = bufSize;
    // 2.初始化逐比特读取工具句柄
    bs_t *bs = bs_new(buffer, bufSize);
    
    // 3. 读取nal header 7.3.1
    nalu->forbidden_zero_bit = bs_read_u(bs, 1);
    nalu->nal_ref_idc = bs_read_u(bs, 2);
    nalu->nal_unit_type = bs_read_u(bs, 5);
    printf("%02X forbidden_zero_bit:%d nal_ref_idc:%d nal_unit_type:%d\n", buffer[0], nalu->forbidden_zero_bit, nalu->nal_ref_idc, nalu->nal_unit_type);

    switch (nalu->nal_unit_type) {
        case H264_NAL_SPS:
            nalu->len = rbsp_trailing_bits_size(nalu);
            parse_sps(bs, sps);
            break;

        case H264_NAL_PPS:
            nalu->len = rbsp_trailing_bits_size(nalu);
            parse_pps(bs, sps, pps);
            break;

        case H264_NAL_SLICE:
        case H264_NAL_IDR_SLICE:
            slice->idr_flag = (nalu->nal_unit_type == H264_NAL_IDR_SLICE);
            slice->nal_ref_idc = nalu->nal_ref_idc;
            nalu->len = rbsp_trailing_bits_size(nalu);
            parse_slice(bs, sps, pps, slice);
            break;
            
        case H264_NAL_DPA:
            nalu->len = rbsp_trailing_bits_size(nalu);
            break;
            
        case H264_NAL_DPB:
            nalu->len = rbsp_trailing_bits_size(nalu);
            break;
            
        case H264_NAL_DPC:
            nalu->len = rbsp_trailing_bits_size(nalu);
            break;
            
        default:
            break;
    }
    free(buffer);
    bs_free(bs);
    return 0;
}
