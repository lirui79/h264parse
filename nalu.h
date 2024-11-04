
#ifndef __NALU_H_
#define __NALU_H_

#include <stdio.h>
#include <stdlib.h>
#include "nalutil.h"
#include "slice.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Network Abstraction Layer (NAL) unit
@see 7.3.1 NAL unit syntax
*/
typedef struct {
    // nal header
    int            forbidden_zero_bit;                     // f(1)
    int            nal_ref_idc;                            // u(2)
    int            nal_unit_type;                          // u(5)
    unsigned int   len;                // 最开始保存nalu_size, 然后保存rbsp_size,最终保存SODB的长度
    uint8_t       *buf;
} nalu_t;



/* 7.4.1 Table 7-1 NAL unit types */
enum nal_unit_type {
    H264_NAL_UNKNOWN         = 0,
    H264_NAL_SLICE           = 1,
    H264_NAL_DPA             = 2,
    H264_NAL_DPB             = 3,
    H264_NAL_DPC             = 4,
    H264_NAL_IDR_SLICE       = 5,
    H264_NAL_SEI             = 6,
    H264_NAL_SPS             = 7,
    H264_NAL_PPS             = 8,
    H264_NAL_AUD             = 9,
    H264_NAL_END_SEQUENCE    = 10,
    H264_NAL_END_STREAM      = 11,
    H264_NAL_FILLER_DATA     = 12,
    H264_NAL_SPS_EXT         = 13,
    H264_NAL_AUXILIARY_SLICE = 19,
};

unsigned int FindStartCode(uint8_t *rawbs);

unsigned int FindStartCodePos(uint8_t *rawbs, unsigned int size, unsigned int startPos);

unsigned int FindNaluPos(uint8_t *rawbs, unsigned int size, unsigned int startPos, nalu_t *nalu);

unsigned int ParseNalu(nalu_t *nalu, sps_t *sps, pps_t *pps, slice_t *slice);

unsigned int ConverNaluToRbsp(const uint8_t *rawbs, unsigned int size, uint8_t* rbbuf, unsigned int* rbsize);

unsigned int ConverRbspToSodb(nalu_t *nalu);

// 在rbsp_trailing_bits()之前是否有更多数据
unsigned int MoreRbspData(bs_t *b);

nalu_t *allocNalu();

void freeNalu(nalu_t *nalu);

#ifdef __cplusplus
}
#endif

#endif /* nalu_h */
