#ifndef ___SLICE__H__
#define ___SLICE__H__

#include <stdio.h>
#include "header.h"
#include "nalutil.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    Slice_Type_P = 0,
    Slice_Type_B,
    Slice_Type_I,
    Slice_Type_SP,
    Slice_Type_SI
} Slice_Type;

typedef struct
{
    int idr_flag; // 是否为IDR帧
    int nal_ref_idc; // nalu->nal_ref_idc
    slice_header_t slice_header;
} slice_t;

void processSlice(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice);

slice_t *allocSlice(void); // 初始化slice结构体

void freeSlice(slice_t *slice); // 释放slice



void processSliceHeader(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice);

void parse_first_three_element(bs_t *b, slice_header_t *slice_header);

void parse_rest_elememt_of_sliceHeader(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice);

void parse_ref_pic_list_modification(bs_t *b, slice_header_t *slice_header);

void alloc_ref_pic_list_modification_buffer(slice_header_t *slice_header);

void parse_pred_weight_table(bs_t *b, sps_t *sps, slice_header_t *slice_header);

void parse_dec_ref_pic_marking(bs_t *b, slice_t *slice);


#ifdef __cplusplus
}
#endif


#endif /* ___SLICE__H__ */
