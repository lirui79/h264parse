#include "avc_stream.h"


/**
 解析pps句法元素
 [h264协议文档位置]：7.3.2.2 Picture parameter set RBSP syntax
 */
int  parse_pps(bs_t *b, sps_t *sps, pps_t *pps) {
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
            int scalingListCycle = 6 + ((chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag;// YUV444==3
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
