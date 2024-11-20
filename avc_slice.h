#ifndef __AVC_SLICE__H_
#define __AVC_SLICE__H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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


/**
 dec_ref_pic_marking()
 [h264协议文档位置]：7.3.3.3 Decoded reference picture marking syntax
 */
typedef struct
{
    int no_output_of_prior_pics_flag;                   // u(1)
    int long_term_reference_flag;                       // u(1)
    int adaptive_ref_pic_marking_mode_flag;             // u(1)
    int memory_management_control_operation[64];    // ue(v)
    int difference_of_pic_nums_minus1[64];      // ue(v)
    int long_term_pic_num[64];                  // ue(v)
    int long_term_frame_idx[64];                // ue(v)
    int max_long_term_frame_idx_plus1[64];      // ue(v)  
} dec_ref_pic_marking_t;

/**
 pred_weight_table()
 [h264协议文档位置]：7.3.3.2 Prediction weight table syntax
 */
typedef struct
{
    int luma_log2_weight_denom;                         // ue(v)
    int chroma_log2_weight_denom;                   // ue(v)
    int luma_weight_l0_flag;                        // u(1)
    int luma_weight_l0[32];                     // se(v)
    int luma_offset_l0[32];                     // se(v)
    int chroma_weight_l0_flag;                  // u(1)
    int chroma_weight_l0[32][2];        // se(v)
    int chroma_offset_l0[32][2];        // se(v)
    int luma_weight_l1_flag;                    // u(1)
    int luma_weight_l1[32];                 // se(v)
    int luma_offset_l1[32];                 // se(v)
    int chroma_weight_l1_flag;              // u(1)
    int chroma_weight_l1[32][2];    // se(v)
    int chroma_offset_l1[32][2];    // se(v)
} pred_weight_table_t;

/**
 2005.03版h264：ref_pic_list_reordering()
 2017.04版h264：ref_pic_list_modification()
 下面以最新版也即2017.04版进行定义
 [h264协议文档位置]：7.3.3.1 Reference picture list modification syntax
 */
typedef struct
{
    int ref_pic_list_modification_flag_l0;        // u(1)
    int *modification_of_pic_nums_idc_lo;         // ue(v)
    int *abs_diff_pic_num_minus1_lo;          // ue(v)
    int *long_term_pic_num_lo;                // ue(v)
    int ref_pic_list_modification_flag_l1;         // u(1)
    int *modification_of_pic_nums_idc_l1;         // ue(v)
    int *abs_diff_pic_num_minus1_l1;          // ue(v)
    int *long_term_pic_num_l1;                // ue(v)
} rplm_t; // ref_pic_list_modification()

/**
 slice_header( )
 [h264协议文档位置]：7.3.3 Slice header syntax
 */
typedef struct
{
    int first_mb_in_slice;                                // ue(v)
    int slice_type;                                       // ue(v)
    int pic_parameter_set_id;                             // ue(v)
    int colour_plane_id;                              // u(2)
    int frame_num;                                        // u(v)
    int field_pic_flag;                               // u(1)
    int bottom_field_flag;                        // u(1)
    int idr_pic_id;                                   // ue(v)
    int pic_order_cnt_lsb;                            // u(v)
    int delta_pic_order_cnt_bottom;               // se(v)
    int delta_pic_order_cnt[2];                       // se(v)
    int redundant_pic_cnt;                            // ue(v)
    int direct_spatial_mv_pred_flag;                  // u(1)
    int num_ref_idx_active_override_flag;             // u(1)
    int num_ref_idx_l0_active_minus1;             // ue(v)
    int num_ref_idx_l1_active_minus1;         // ue(v)
    rplm_t ref_pic_list_modification;
    pred_weight_table_t pred_weight_table;
    dec_ref_pic_marking_t dec_ref_pic_marking;
    int cabac_init_idc;                               // ue(v)
    int slice_qp_delta;                                   // se(v)
    int sp_for_switch_flag;                       // u(1)
    int slice_qs_delta;                               // se(v)
    int disable_deblocking_filter_idc;                // ue(v)
    int slice_alpha_c0_offset_div2;               // se(v)
    int slice_beta_offset_div2;                   // se(v)
    int slice_group_change_cycle;                     // u(v)
} slice_header_t;


typedef struct
{
    int idr_flag; // 是否为IDR帧
    int nal_ref_idc; // nalu->nal_ref_idc
    slice_header_t slice_header;
} slice_t;


#ifdef __cplusplus
}
#endif

#endif//__AVC_SLICE__H_