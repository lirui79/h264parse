#ifndef __AVC_SPS_H_
#define __AVC_SPS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 hrd_parameters()
 @see Annex E E.1.2 HRD parameters syntax
 */
typedef struct
{
    int cpb_cnt_minus1;                                 // ue(v)
    int bit_rate_scale;                                 // u(4)
    int cpb_size_scale;                                 // u(4)
    int bit_rate_value_minus1[32];         // ue(v)
    int cpb_size_value_minus1[32];         // ue(v)
    int cbr_flag[32];                      // u(1)
    int initial_cpb_removal_delay_length_minus1;        // u(5)
    int cpb_removal_delay_length_minus1;                // u(5)
    int dpb_output_delay_length_minus1;                 // u(5)
    int time_offset_length;                             // u(5)
}hrd_parameters_t;

/**
 vui_parameters()
 @see Annex E E.1.1 VUI parameters syntax
 */
typedef struct
{
    int aspect_ratio_info_present_flag;                 // u(1)
    int aspect_ratio_idc;                           // u(8)
    int sar_width;                              // u(16)
    int sar_height;                             // u(16)

    int overscan_info_present_flag;                     // u(1)
    int overscan_appropriate_flag;                  // u(1)
    int video_signal_type_present_flag;                 // u(1)
    int video_format;                               // u(3)
    int video_full_range_flag;                      // u(1)
    int colour_description_present_flag;            // u(1)
    int colour_primaries;                       // u(8)
    int transfer_characteristics;               // u(8)
    int matrix_coefficients;                    // u(8)

    int chroma_loc_info_present_flag;                   // u(1)
    int chroma_sample_loc_type_top_field;           // ue(v)
    int chroma_sample_loc_type_bottom_field;        // ue(v)

    int timing_info_present_flag;                       // u(1)
    int num_units_in_tick;                          // u(32)
    int time_scale;                                 // u(32)
    int fixed_frame_rate_flag;                      // u(1)

    int nal_hrd_parameters_present_flag;                // u(1)
    hrd_parameters_t nal_hrd_parameters;

    int vcl_hrd_parameters_present_flag;                // u(1)
    hrd_parameters_t vcl_hrd_parameters;
    int low_delay_hrd_flag;                         // u(1)

    int pic_struct_present_flag;                        // u(1)
    int bitstream_restriction_flag;                     // u(1)
    int motion_vectors_over_pic_boundaries_flag;    // u(1)
    int max_bytes_per_pic_denom;                    // ue(v)
    int max_bits_per_mb_denom;                      // ue(v)
    int log2_max_mv_length_horizontal;              // ue(v)
    int log2_max_mv_length_vertical;                // ue(v)
    int max_num_reorder_frames;                     // ue(v)
    int max_dec_frame_buffering;                    // ue(v)
}vui_parameters_t;

/**
 Sequence Parameter Set
 @see 7.3.2.1 Sequence parameter set RBSP syntax
 */
typedef struct
{
    int profile_idc;                                    // u(8)
    /*  —————————— 编码级别的制约条件 Start  —————————— */
    int constraint_set0_flag;                           // u(1)
    int constraint_set1_flag;                           // u(1)
    int constraint_set2_flag;                           // u(1)
    int constraint_set3_flag;                           // u(1)
    int constraint_set4_flag;                           // u(1)
    int constraint_set5_flag;                           // u(1)
    int reserved_zero_2bits;                            // u(2)
    /*  —————————— 编码级别的制约条件 End  —————————— */

    int level_idc;                                      // u(8)
    int seq_parameter_set_id;                           // ue(v)

    /*  —————————— 几个罕见级别对应的句法元素 Start  —————————— */
    int chroma_format_idc;                                  // ue(v)
    int separate_colour_plane_flag;                     // u(1)
    int bit_depth_luma_minus8;                              // ue(v)
    int bit_depth_chroma_minus8;                            // ue(v)
    int qpprime_y_zero_transform_bypass_flag;               // u(1)

    int seq_scaling_matrix_present_flag;                    // u(1)
    int seq_scaling_list_present_flag[12]; // 最大值12数组   // u(1)
    int ScalingList4x4[6][16]; // 二维数组遍历
    int UseDefaultScalingMatrix4x4Flag[6];
    int ScalingList8x8[6][64];
    int UseDefaultScalingMatrix8x8Flag[6];
    /*  —————————— 几个罕见级别对应的句法元素 End  —————————— */

    /*  —————————— 用来计算POC的句法元素 Start  —————————— */
    int log2_max_frame_num_minus4;                              // ue(v)
    int pic_order_cnt_type;                                     // ue(v)
    int log2_max_pic_order_cnt_lsb_minus4;                  // ue(v)
    int delta_pic_order_always_zero_flag;                   // u(1)
    int offset_for_non_ref_pic;                             // se(v)
    int offset_for_top_to_bottom_field;                     // se(v)
    int num_ref_frames_in_pic_order_cnt_cycle;              // ue(v)
    int offset_for_ref_frame[256]; // 最大值256数组       // se(v)
    /*  —————————— 用来计算POC的句法元素 End  —————————— */

    int max_num_ref_frames;                                     // ue(v)
    int gaps_in_frame_num_value_allowed_flag;                   // u(1)

    /*  —————————— 图像宽高相关 Start  —————————— */
    int pic_width_in_mbs_minus1;                                // ue(v)
    int pic_height_in_map_units_minus1;                         // ue(v)
    int frame_mbs_only_flag;                                    // u(1)
    int mb_adaptive_frame_field_flag;                       // u(1)
    /*  —————————— 图像宽高相关 End  —————————— */

    int direct_8x8_inference_flag;                              // u(1)

    /*  —————————— 解码后图像剪裁的几个句法元素 Start  —————————— */
    int frame_cropping_flag;                                    // u(1)
    int frame_crop_left_offset;                             // ue(v)
    int frame_crop_right_offset;                            // ue(v)
    int frame_crop_top_offset;                              // ue(v)
    int frame_crop_bottom_offset;                           // ue(v)
    /*  —————————— 解码后图像剪裁的几个句法元素 End  —————————— */

    int vui_parameters_present_flag;                            // u(1)
    vui_parameters_t vui_parameters; // Annex E E.1.1
} sps_t;


#ifdef __cplusplus
}
#endif

#endif//