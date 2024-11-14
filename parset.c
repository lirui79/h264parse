#include "parset.h"
#include "frame.h"
#include "nalu.h"

// 因为sps_id的取值范围为[0,31]，因此数组容量最大为32，详见7.4.2.1
//static sps_t Sequence_Parameters_Set_Array[32];
// 因为pps_id的取值范围为[0,255]，因此数组容量最大为256，详见7.4.2.2
//static pps_t Picture_Parameters_Set_Array[256];


void scaling_list(bs_t *b, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag);
void parse_vui_parameters(bs_t *b, sps_t *sps);
void parse_vui_hrd_parameters(bs_t *b, hrd_parameters_t *hrd);

/**
 解析sps句法元素
 [h264协议文档位置]：7.3.2.1.1 Sequence parameter set data syntax
 */
int processSPS(bs_t *b, sps_t *sps) {
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
        if (sps->chroma_format_idc == YUV_4_4_4) {
            sps->separate_colour_plane_flag = bs_read_u(b, 1);
        }
        sps->bit_depth_luma_minus8 = bs_read_ue(b);
        sps->bit_depth_chroma_minus8 = bs_read_ue(b);
        sps->qpprime_y_zero_transform_bypass_flag = bs_read_u(b, 1);
        sps->seq_scaling_matrix_present_flag = bs_read_u(b, 1);
        
        if (sps->seq_scaling_matrix_present_flag) {
            int scalingListCycle = (sps->chroma_format_idc != YUV_4_4_4) ? 8 : 12;
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
 解析vui_parameters()句法元素
 [h264协议文档位置]：Annex E.1.1
 */
void parse_vui_parameters(bs_t *b, sps_t *sps) {
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
 解析hrd_parameters()句法元素
 [h264协议文档位置]：Annex E.1.2
 */
void parse_vui_hrd_parameters(bs_t *b, hrd_parameters_t *hrd) {
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
 解析pps句法元素
 [h264协议文档位置]：7.3.2.2 Picture parameter set RBSP syntax
 */
int  processPPS(bs_t *b, sps_t *sps, pps_t *pps) {
    // 解析slice_group_id[]需用的比特个数
    int bitsNumberOfEachSliceGroupID;
    
    pps->pic_parameter_set_id = bs_read_ue(b);
    pps->seq_parameter_set_id = bs_read_ue(b);
    pps->entropy_coding_mode_flag = bs_read_u(b, 1);
    pps->bottom_field_pic_order_in_frame_present_flag = bs_read_u(b, 1);
    
    /*  —————————— FMO相关 Start  —————————— */
    pps->num_slice_groups_minus1 = bs_read_ue(b);
    if (pps->num_slice_groups_minus1 > 0) {
        pps->slice_group_map_type = bs_read_ue(b);
        if (pps->slice_group_map_type == 0)
        {
            for (int i = 0; i <= pps->num_slice_groups_minus1; i++) {
                pps->run_length_minus1[i] = bs_read_ue(b);
            }
        }
        else if (pps->slice_group_map_type == 2)
        {
            for (int i = 0; i < pps->num_slice_groups_minus1; i++) {
                pps->top_left[i] = bs_read_ue(b);
                pps->bottom_right[i] = bs_read_ue(b);
            }
        }
        else if (pps->slice_group_map_type == 3 ||
                 pps->slice_group_map_type == 4 ||
                 pps->slice_group_map_type == 5)
        {
            pps->slice_group_change_direction_flag = bs_read_u(b, 1);
            pps->slice_group_change_rate_minus1 = bs_read_ue(b);
        }
        else if (pps->slice_group_map_type == 6)
        {
            // 1.计算解析slice_group_id[]需用的比特个数，Ceil( Log2( num_slice_groups_minus1 + 1 ) )
            if (pps->num_slice_groups_minus1+1 >4)
                bitsNumberOfEachSliceGroupID = 3;
            else if (pps->num_slice_groups_minus1+1 > 2)
                bitsNumberOfEachSliceGroupID = 2;
            else
                bitsNumberOfEachSliceGroupID = 1;
            
            // 2.动态初始化指针pps->slice_group_id
            pps->pic_size_in_map_units_minus1 = bs_read_ue(b);
            pps->slice_group_id = calloc(pps->pic_size_in_map_units_minus1+1, 1);
            if (pps->slice_group_id == NULL) {
                fprintf(stderr, "%s\n", "parse_pps_syntax_element slice_group_id Error");
                exit(-1);
            }
            
            for (int i = 0; i <= pps->pic_size_in_map_units_minus1; i++) {
                pps->slice_group_id[i] = bs_read_u(b, bitsNumberOfEachSliceGroupID);
            }
        }
    }
    /*  —————————— FMO相关 End  —————————— */
    
    pps->num_ref_idx_l0_default_active_minus1 = bs_read_ue(b);
    pps->num_ref_idx_l1_default_active_minus1 = bs_read_ue(b);
    
    pps->weighted_pred_flag = bs_read_u(b, 1);
    pps->weighted_bipred_idc = bs_read_u(b, 2);
    
    pps->pic_init_qp_minus26 = bs_read_se(b);
    pps->pic_init_qs_minus26 = bs_read_se(b);
    pps->chroma_qp_index_offset = bs_read_se(b);
    
    pps->deblocking_filter_control_present_flag = bs_read_u(b, 1);
    pps->constrained_intra_pred_flag = bs_read_u(b, 1);
    pps->redundant_pic_cnt_present_flag = bs_read_u(b, 1);
    
    // 如果有更多rbsp数据
    if (more_rbsp_data(b)) {
        pps->transform_8x8_mode_flag = bs_read_u(b, 1);
        pps->pic_scaling_matrix_present_flag = bs_read_u(b, 1);
        if (pps->pic_scaling_matrix_present_flag) {
            int chroma_format_idc = sps->chroma_format_idc;//Sequence_Parameters_Set_Array[pps->seq_parameter_set_id].chroma_format_idc;
            int scalingListCycle = 6 + ((chroma_format_idc != YUV_4_4_4) ? 2 : 6) * pps->transform_8x8_mode_flag;
            for (int i = 0; i < scalingListCycle; i++) {
                pps->pic_scaling_list_present_flag[i] = bs_read_u(b, 1);
                if (pps->pic_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        scaling_list(pps->ScalingList4x4[i], 16, &pps->UseDefaultScalingMatrix4x4Flag[i], b);
                    }else {
                        scaling_list(pps->ScalingList8x8[i-6], 64, &pps->UseDefaultScalingMatrix8x8Flag[i], b);
                    }
                }
            }
        }
        pps->second_chroma_qp_index_offset = bs_read_se(b);
    }else {
        pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset;
    }
    
    return 0;
}

//void save_sps_as_available(sps_t *sps)
//{
//    memcpy (&Sequence_Parameters_Set_Array[sps->seq_parameter_set_id], sps, sizeof (sps_t));
//}

/*
void save_pps_as_available(pps_t *pps)
{
    // 2.更新同一个pps_id对应的pps时再释放
    if (Picture_Parameters_Set_Array[pps->pic_parameter_set_id].slice_group_id != NULL) {
        free(Picture_Parameters_Set_Array[pps->pic_parameter_set_id].slice_group_id);
    }
    
    // 0.保存sps
    memcpy (&Picture_Parameters_Set_Array[pps->pic_parameter_set_id], pps, sizeof (pps_t));
 
    // 1.不释放由pps->slice_group_id指向的内存，交由Picture_Parameters_Set_Array[pps->pps_id]使用
    Picture_Parameters_Set_Array[pps->pic_parameter_set_id].slice_group_id = pps->slice_group_id;
    pps->slice_group_id = NULL;
}//*/

/*
void activeParameterSet(int pps_id)
{
    active_pps = &Picture_Parameters_Set_Array[pps_id];
    active_sps = &Sequence_Parameters_Set_Array[active_pps->seq_parameter_set_id];
}//*/

// 初始化sps结构体
sps_t *allocSPS(void)
{
    sps_t *sps = (sps_t*) malloc(sizeof(sps_t));
    if (sps == NULL) {
        fprintf(stderr, "%s\n", "Alloc SPS Error");
        return NULL;
    }
    memset(sps, 0, sizeof(sps_t));
    return sps;
}

// 释放sps
void freeSPS(sps_t *sps)
{
    free(sps);
}

// 初始化pps结构体
pps_t *allocPPS(void)
{
    pps_t *pps = (pps_t*) malloc(sizeof(pps_t));
    if (pps == NULL) {
        fprintf(stderr, "%s\n", "Alloc PPS Error");
        return NULL;
    }
    memset(pps, 0, sizeof(pps_t));
    pps->slice_group_id = NULL;
    return pps;
}

// 释放pps
void freePPS(pps_t *pps)
{
    if (pps->slice_group_id != NULL) {
        free (pps->slice_group_id);
    }
    free(pps);
}
