#ifndef __AVC_PPS_H_
#define __AVC_PPS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 Picture Parameter Set
 @see 7.3.2.2 Picture parameter set RBSP syntax
 */
typedef struct
{
    int pic_parameter_set_id;                            // ue(v)
    int seq_parameter_set_id;                            // ue(v)
    int entropy_coding_mode_flag;                        // u(1)
    int bottom_field_pic_order_in_frame_present_flag;    // u(1)

    /*  —————————— FMO相关 Start  —————————— */
    int num_slice_groups_minus1;                         // ue(v)
    int slice_group_map_type;                        // ue(v)
    // num_slice_groups_minus1取值范围[0, 7]，见附录A
    int run_length_minus1[8];                // ue(v)
    int top_left[8];                         // ue(v)
    int bottom_right[8];                     // ue(v)
    int slice_group_change_direction_flag;       // u(1)
    int slice_group_change_rate_minus1;          // ue(v)
    int pic_size_in_map_units_minus1;            // ue(v)
    int *slice_group_id;                     // u(v)
    /*  —————————— FMO相关 End  —————————— */

    int num_ref_idx_l0_default_active_minus1;            // ue(v)
    int num_ref_idx_l1_default_active_minus1;            // ue(v)
    int weighted_pred_flag;                              // u(1)
    int weighted_bipred_idc;                             // u(2)

    int pic_init_qp_minus26;                             // se(v)
    int pic_init_qs_minus26;                             // se(v)
    int chroma_qp_index_offset;                          // se(v)

    int deblocking_filter_control_present_flag;          // u(1)
    int constrained_intra_pred_flag;                     // u(1)
    int redundant_pic_cnt_present_flag;                  // u(1)

    int transform_8x8_mode_flag;                     // u(1)
    int pic_scaling_matrix_present_flag;             // u(1)
    int pic_scaling_list_present_flag[12];    // u(1)
    int ScalingList4x4[6][16]; // 二维数组遍历
    int UseDefaultScalingMatrix4x4Flag[6];
    int ScalingList8x8[6][64];
    int UseDefaultScalingMatrix8x8Flag[6];
    int second_chroma_qp_index_offset;                  // se(v)
} pps_t;



#ifdef __cplusplus
}
#endif

#endif//