#include "avc_stream.h"


/**
 解析hrd_parameters()句法元素
 [h264协议文档位置]：Annex E.1.2
 */
static void parse_vui_hrd_parameters(bs_t *b, hrd_parameters_t *hrd) {
    hrd->cpb_cnt_minus1 = bs_read_ue(b);
    hrd->bit_rate_scale = bs_read_u(b, 4);
    hrd->cpb_size_scale = bs_read_u(b, 4);
    
    for (int SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++) {
        hrd->bit_rate_value_minus1[SchedSelIdx] = bs_read_ue(b);
        hrd->cpb_size_value_minus1[SchedSelIdx] = bs_read_ue(b);
        hrd->cbr_flag[SchedSelIdx] = bs_read_u(b, 1);
    }
    
    hrd->initial_cpb_removal_delay_length_minus1 = bs_read_u(b, 5);
    hrd->cpb_removal_delay_length_minus1 = bs_read_u(b, 5);
    hrd->dpb_output_delay_length_minus1 = bs_read_u(b, 5);
    hrd->time_offset_length = bs_read_u(b, 5);
}

/**
 解析vui_parameters()句法元素
 [h264协议文档位置]：Annex E.1.1
 */
static void parse_vui_parameters(bs_t *b, sps_t *sps) {
    sps->vui_parameters.aspect_ratio_info_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.aspect_ratio_info_present_flag) {
        sps->vui_parameters.aspect_ratio_idc = bs_read_u(b, 8);
        if (sps->vui_parameters.aspect_ratio_idc == 255) { // Extended_SAR值为255
            sps->vui_parameters.sar_width = bs_read_u(b, 16);
            sps->vui_parameters.sar_height = bs_read_u(b, 16);
        }
    }
    
    sps->vui_parameters.overscan_info_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.overscan_info_present_flag) {
        sps->vui_parameters.overscan_appropriate_flag = bs_read_u(b, 1);
    }
    
    sps->vui_parameters.video_signal_type_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.video_signal_type_present_flag) {
        sps->vui_parameters.video_format = bs_read_u(b, 3);
        sps->vui_parameters.video_full_range_flag = bs_read_u(b, 1);
        
        sps->vui_parameters.colour_description_present_flag = bs_read_u(b, 1);
        if (sps->vui_parameters.colour_description_present_flag) {
            sps->vui_parameters.colour_primaries = bs_read_u(b, 8);
            sps->vui_parameters.transfer_characteristics = bs_read_u(b, 8);
            sps->vui_parameters.matrix_coefficients = bs_read_u(b, 8);
        }
    }
    
    sps->vui_parameters.chroma_loc_info_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.chroma_loc_info_present_flag) {
        sps->vui_parameters.chroma_sample_loc_type_top_field = bs_read_ue(b);
        sps->vui_parameters.chroma_sample_loc_type_bottom_field = bs_read_ue(b);
    }
    
    sps->vui_parameters.timing_info_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.timing_info_present_flag) {
        sps->vui_parameters.num_units_in_tick = bs_read_u(b, 32);
        sps->vui_parameters.time_scale = bs_read_u(b, 32);
        sps->vui_parameters.fixed_frame_rate_flag = bs_read_u(b, 1);
    }
    
    sps->vui_parameters.nal_hrd_parameters_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.nal_hrd_parameters_present_flag) {
        parse_vui_hrd_parameters(b, &sps->vui_parameters.nal_hrd_parameters);
    }
    
    sps->vui_parameters.vcl_hrd_parameters_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.vcl_hrd_parameters_present_flag) {
        parse_vui_hrd_parameters(b, &sps->vui_parameters.vcl_hrd_parameters);
    }
    
    if (sps->vui_parameters.nal_hrd_parameters_present_flag ||
        sps->vui_parameters.vcl_hrd_parameters_present_flag) {
        sps->vui_parameters.low_delay_hrd_flag = bs_read_u(b, 1);
    }
    
    sps->vui_parameters.pic_struct_present_flag = bs_read_u(b, 1);
    sps->vui_parameters.bitstream_restriction_flag = bs_read_u(b, 1);
    if (sps->vui_parameters.bitstream_restriction_flag) {
        sps->vui_parameters.motion_vectors_over_pic_boundaries_flag = bs_read_u(b, 1);
        sps->vui_parameters.max_bytes_per_pic_denom = bs_read_ue(b);
        sps->vui_parameters.max_bits_per_mb_denom = bs_read_ue(b);
        sps->vui_parameters.log2_max_mv_length_horizontal = bs_read_ue(b);
        sps->vui_parameters.log2_max_mv_length_vertical = bs_read_ue(b);
        sps->vui_parameters.max_num_reorder_frames = bs_read_ue(b);
        sps->vui_parameters.max_dec_frame_buffering = bs_read_ue(b);
    }
}


/**
 解析sps句法元素
 [h264协议文档位置]：7.3.2.1.1 Sequence parameter set data syntax
 详见7.4.2.1
 */
int parse_sps(bs_t *b, sps_t *sps) {
    int PicWidthInSamplesL = 0, PicHeightInSamplesL = 0;
    sps->profile_idc = bs_read_u(b, 8);
    sps->constraint_set0_flag = bs_read_u(b, 1);
    sps->constraint_set1_flag = bs_read_u(b, 1);
    sps->constraint_set2_flag = bs_read_u(b, 1);
    sps->constraint_set3_flag = bs_read_u(b, 1);
    sps->constraint_set4_flag = bs_read_u(b, 1);
    sps->constraint_set5_flag = bs_read_u(b, 1);
    sps->reserved_zero_2bits = bs_read_u(b, 2);
    sps->level_idc = bs_read_u(b, 8);
    
    sps->seq_parameter_set_id = bs_read_ue(b);
    
    if (sps->profile_idc == 100 || sps->profile_idc == 110 || sps->profile_idc == 122 || 
        sps->profile_idc == 244 || sps->profile_idc == 44 || sps->profile_idc == 83 || 
        sps->profile_idc == 86 || sps->profile_idc == 118 || sps->profile_idc == 128 || 
        sps->profile_idc == 138 || sps->profile_idc == 139 || sps->profile_idc == 134 || sps->profile_idc == 135) {
        
        sps->chroma_format_idc = bs_read_ue(b);
        if (sps->chroma_format_idc == 3) {//YUV444==3
            sps->separate_colour_plane_flag = bs_read_u(b, 1);
        }
        sps->bit_depth_luma_minus8 = bs_read_ue(b);
        sps->bit_depth_chroma_minus8 = bs_read_ue(b);
        sps->qpprime_y_zero_transform_bypass_flag = bs_read_u(b, 1);
        sps->seq_scaling_matrix_present_flag = bs_read_u(b, 1);
        
        if (sps->seq_scaling_matrix_present_flag) {
            int scalingListCycle = (sps->chroma_format_idc != 3) ? 8 : 12;//YUV444==3
            for (int i = 0; i < scalingListCycle; i++) {
                sps->seq_scaling_list_present_flag[i] = bs_read_u(b, 1);
                if (sps->seq_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        scaling_list(b, sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i]);
                    }else {
                        scaling_list(b, sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6]);
                    }
                }
            }
        }
    }
    
    sps->log2_max_frame_num_minus4 = bs_read_ue(b);
    sps->pic_order_cnt_type = bs_read_ue(b);
    if (sps->pic_order_cnt_type == 0) {
        sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(b);
    }else if (sps->pic_order_cnt_type == 1) {
        sps->delta_pic_order_always_zero_flag = bs_read_u(b, 1);
        sps->offset_for_non_ref_pic = bs_read_se(b);
        sps->offset_for_top_to_bottom_field = bs_read_se(b);
        sps->num_ref_frames_in_pic_order_cnt_cycle = bs_read_ue(b);
        for (int i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
            sps->offset_for_ref_frame[i] = bs_read_se(b);
        }
    }
    
    sps->max_num_ref_frames = bs_read_ue(b);
    sps->gaps_in_frame_num_value_allowed_flag = bs_read_u(b, 1);
    
    sps->pic_width_in_mbs_minus1 = bs_read_ue(b);
    sps->pic_height_in_map_units_minus1 = bs_read_ue(b);
    sps->frame_mbs_only_flag = bs_read_u(b, 1);
    if (!sps->frame_mbs_only_flag) {
        sps->mb_adaptive_frame_field_flag = bs_read_u(b, 1);
    }
    
    sps->direct_8x8_inference_flag = bs_read_u(b, 1);
    
    sps->frame_cropping_flag = bs_read_u(b, 1);
    if (sps->frame_cropping_flag) {
        sps->frame_crop_left_offset = bs_read_ue(b);
        sps->frame_crop_right_offset = bs_read_ue(b);
        sps->frame_crop_top_offset = bs_read_ue(b);
        sps->frame_crop_bottom_offset = bs_read_ue(b);
    }
    
    sps->vui_parameters_present_flag = bs_read_u(b, 1);
    if (sps->vui_parameters_present_flag) {
        parse_vui_parameters(b, sps);
    }

    printf("crop(%d %d %d %d)\n", sps->frame_crop_left_offset, sps->frame_crop_right_offset, sps->frame_crop_top_offset, sps->frame_crop_bottom_offset);

    PicWidthInSamplesL = 16 * (sps->pic_width_in_mbs_minus1 + 1);
    PicHeightInSamplesL =  16 * (sps->pic_height_in_map_units_minus1 + 1);
    printf("pic_width_in_mbs_minus1:%d pic_height_in_map_units_minus1:%d %d %d\n", sps->pic_width_in_mbs_minus1, sps->pic_height_in_map_units_minus1, PicWidthInSamplesL, PicHeightInSamplesL);
    return 0;
}
