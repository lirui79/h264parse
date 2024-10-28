
#include "header.h"
#include "slice.h"
#include "parset.h"
#include "frame.h"

/**
 处理slice_header，包含三步：
 1.先解析头三个元素
 2.去激活参数集
 3.解析剩余的句法元素
 */
void processSliceHeader(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice) {
    slice_header_t *slice_header = &slice->slice_header;
    // 0.解析前三个句法元素
    parse_first_three_element(b, slice_header);
    // 1.激活参数集
//    activeParameterSet(slice_header->pic_parameter_set_id);
    // 2.解析剩余的句法元素
    parse_rest_elememt_of_sliceHeader(b, sps, pps, slice);
}

/**
 解析slice_header头三个句法元素
 [h264协议文档位置]：7.3.3 Slice header syntax
 */
void parse_first_three_element(bs_t *b, slice_header_t *slice_header) {
    slice_header->first_mb_in_slice = bs_read_ue(b);
    
    // 因为slice_type值为0~9，0~4和5~9重合
    int slice_type = bs_read_ue(b);
    if (slice_type > 4) {slice_type -= 5;}
    slice_header->slice_type = slice_type;
	printf("mb_in_slice:slice_type %d:%d \n", slice_header->first_mb_in_slice, slice_header->slice_type);
    
    slice_header->pic_parameter_set_id = bs_read_ue(b);
}

/**
 解析slice_header剩余句法元素
 [h264协议文档位置]：7.3.3 Slice header syntax
 */
void parse_rest_elememt_of_sliceHeader(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice) {
    slice_header_t *slice_header = &slice->slice_header;
    if (sps->separate_colour_plane_flag == 1) {
        slice_header->colour_plane_id = bs_read_u(b, 2);
    }else {
        slice_header->colour_plane_id = COLOR_PLANE_Y;
    }
    
    slice_header->frame_num = bs_read_u(b, sps->log2_max_frame_num_minus4 + 4);
    
    // FIXME: frame_num gap processing
    
    if (sps->frame_mbs_only_flag) {
        slice_header->field_pic_flag = 0;
    } else {
        slice_header->field_pic_flag = bs_read_u(b, 1);
        if (slice_header->field_pic_flag) {
            slice_header->bottom_field_flag = bs_read_u(b, 1);
        }else {
            slice_header->bottom_field_flag = 0;
        }
    }
    
    if (slice->idr_flag) {
        slice_header->idr_pic_id = bs_read_ue(b);
    }
    
    if (sps->pic_order_cnt_type == 0)
    {
        slice_header->pic_order_cnt_lsb = bs_read_u(b, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if (pps->bottom_field_pic_order_in_frame_present_flag && !slice_header->field_pic_flag) {
            slice_header->delta_pic_order_cnt_bottom = bs_read_se(b);
        }else {
            slice_header->delta_pic_order_cnt_bottom = 0;
        }
    }
    
    if (sps->pic_order_cnt_type == 1 &&
        !sps->delta_pic_order_always_zero_flag)
    {
        slice_header->delta_pic_order_cnt[0] = bs_read_se(b);
        if (pps->bottom_field_pic_order_in_frame_present_flag &&
            !slice_header->field_pic_flag) {
            slice_header->delta_pic_order_cnt[1] = bs_read_se(b);
        }else {
            slice_header->delta_pic_order_cnt[1] = 0;
        }
    }else if (sps->pic_order_cnt_type == 1) {
        slice_header->delta_pic_order_cnt[0] = 0;
        slice_header->delta_pic_order_cnt[1] = 0;
    }
    
    if (pps->redundant_pic_cnt_present_flag) {
        slice_header->redundant_pic_cnt = bs_read_ue(b);
    }
    
    if (slice_header->slice_type == Slice_Type_B) {
        slice_header->direct_spatial_mv_pred_flag = bs_read_u(b, 1);
    }
    
    if (slice_header->slice_type == Slice_Type_P ||
             slice_header->slice_type == Slice_Type_SP ||
             slice_header->slice_type == Slice_Type_B)
    {
        slice_header->num_ref_idx_active_override_flag = bs_read_u(b, 1);
        if (slice_header->num_ref_idx_active_override_flag)
        {
            slice_header->num_ref_idx_l0_active_minus1 = bs_read_ue(b);
            if (slice_header->slice_type == Slice_Type_B)
            {
                slice_header->num_ref_idx_l1_active_minus1 = bs_read_ue(b);
            }
        }
    }
    
    // 1.解析参考图像列表修正句法元素
    parse_ref_pic_list_modification(b, slice_header);
    
    if ((pps->weighted_pred_flag && (slice_header->slice_type == Slice_Type_P || slice_header->slice_type == Slice_Type_SP)) ||
        (pps->weighted_bipred_idc == 1 && slice_header->slice_type == Slice_Type_B)) {
        // 2.解析预测加权表格句法元素
        parse_pred_weight_table(b, sps, slice_header);
    }
    
    if (slice->nal_ref_idc != 0) {
        // 3.解析解码参考图像标识句法元素
        parse_dec_ref_pic_marking(b, slice);
    }
    
    if (pps->entropy_coding_mode_flag &&
        slice_header->slice_type != Slice_Type_I &&
        slice_header->slice_type != Slice_Type_SI) {
        slice_header->cabac_init_idc = bs_read_ue(b);
    }else {
        slice_header->cabac_init_idc = 0;
    }
    
    slice_header->slice_qp_delta = bs_read_se(b);
    if (slice_header->slice_type == Slice_Type_SP ||
        slice_header->slice_type == Slice_Type_SI) {
        if (slice_header->slice_type == Slice_Type_SP) {
            slice_header->sp_for_switch_flag = bs_read_u(b, 1);
        }
        slice_header->slice_qs_delta = bs_read_se(b);
    }
    
    if (pps->deblocking_filter_control_present_flag) {
        slice_header->disable_deblocking_filter_idc = bs_read_ue(b);
        if (slice_header->disable_deblocking_filter_idc != 1) {
            slice_header->slice_alpha_c0_offset_div2 = bs_read_se(b);
            slice_header->slice_beta_offset_div2 = bs_read_se(b);
        }else {
            // 设置默认值
            slice_header->slice_alpha_c0_offset_div2 = 0;
            slice_header->slice_beta_offset_div2 = 0;
        }
    }else {
        // 设置默认值
        slice_header->disable_deblocking_filter_idc = 0;
        slice_header->slice_alpha_c0_offset_div2 = 0;
        slice_header->slice_beta_offset_div2 = 0;
    }
    
    if (pps->num_slice_groups_minus1 > 0 &&
        pps->slice_group_map_type >= 3 &&
        pps->slice_group_map_type <= 5) {
        // 见7.4.3 slice_header语义
        // 不能直接用pps->pic_size_in_map_units_minus1，因为它可能没值
        int bit_len = ((sps->pic_width_in_mbs_minus1 + 1) * (sps->pic_height_in_map_units_minus1 + 1)) / (pps->slice_group_change_rate_minus1 + 1);
        // 计算Ceil(bit_len)
        if (((sps->pic_width_in_mbs_minus1 + 1) * (sps->pic_height_in_map_units_minus1 + 1)) % (pps->slice_group_change_rate_minus1 + 1)) {
            bit_len++;
        }
        
        // 去计算Ceil( Log2( PicSizeInMapUnits ÷ SliceGroupChangeRate + 1 ) )
        bit_len = calculateCeilLog2(bit_len + 1);
        
        slice_header->slice_group_change_cycle = bs_read_u(b, bit_len);
    }
}

/**
 解析ref_pic_list_modification()句法元素
 [h264协议文档位置]：7.3.3.1 Reference picture list modification syntax
 */
void parse_ref_pic_list_modification(bs_t *b, slice_header_t *slice_header) {
    rplm_t *rplm = &slice_header->ref_pic_list_modification;
    int i, val;
    
    // 0.初始化内存空间
    alloc_ref_pic_list_modification_buffer(slice_header);
    
    if (slice_header->slice_type != Slice_Type_I &&
        slice_header->slice_type != Slice_Type_SI) {
        rplm->ref_pic_list_modification_flag_l0 = bs_read_u(b, 1);
        if (rplm->ref_pic_list_modification_flag_l0) {
            i = 0;
            do {
                val = rplm->modification_of_pic_nums_idc_lo[i] = bs_read_ue(b);
                if (val == 0 || val == 1) {
                    rplm->abs_diff_pic_num_minus1_lo[i] = bs_read_ue(b);
                }else if (val == 2) {
                    rplm->long_term_pic_num_lo[i] = bs_read_ue(b);
                }
                i++;
            } while (val != 3);
        }
    }
    
    if (slice_header->slice_type == Slice_Type_B) {
        rplm->ref_pic_list_modification_flag_l1 = bs_read_u(b, 1);
        if (rplm->ref_pic_list_modification_flag_l1) {
            i = 0;
            do {
                val = rplm->modification_of_pic_nums_idc_l1[i] = bs_read_ue(b);
                if (val == 0 || val == 1) {
                    rplm->abs_diff_pic_num_minus1_l1[i] = bs_read_ue(b);
                }else if (val == 2) {
                    rplm->long_term_pic_num_l1[i] = bs_read_ue(b);
                }
                i++;
            } while (val != 3);
        }
    }
}

/**
 为ref_pic_list_modification()初始化内存空间
 [h264协议文档位置]：7.3.3.1 Reference picture list modification syntax
 */
void alloc_ref_pic_list_modification_buffer(slice_header_t *slice_header) {
    rplm_t *rplm = &slice_header->ref_pic_list_modification;
    
    // 先对参考图像列表0进行重排序，因此重排序的图像个数最多为ref_idx_l0的长度，也即num_ref_idx_l0_active_minus1+1
    int size = slice_header->num_ref_idx_l0_active_minus1 + 1;
    
    if (slice_header->slice_type != Slice_Type_I &&
        slice_header->slice_type != Slice_Type_SI)
    {
        rplm->modification_of_pic_nums_idc_lo = calloc(size, sizeof(int));
        rplm->abs_diff_pic_num_minus1_lo = calloc(size, sizeof(int));
        rplm->long_term_pic_num_lo = calloc(size, sizeof(int));
        
        if (rplm->modification_of_pic_nums_idc_lo == NULL ||
            rplm->abs_diff_pic_num_minus1_lo == NULL ||
            rplm->long_term_pic_num_lo == NULL) {
            fprintf(stderr, "%s\n", "Alloc ref_pic_list_modification_lo Error");
            exit(-1);
        }
    }else {
        rplm->modification_of_pic_nums_idc_lo = NULL;
        rplm->abs_diff_pic_num_minus1_lo = NULL;
        rplm->long_term_pic_num_lo = NULL;
    }
    
    size = slice_header->num_ref_idx_l1_active_minus1 + 1;
    
    if (slice_header->slice_type == Slice_Type_B)
    {
        rplm->modification_of_pic_nums_idc_l1 = calloc(size, sizeof(int));
        rplm->abs_diff_pic_num_minus1_l1 = calloc(size, sizeof(int));
        rplm->long_term_pic_num_l1 = calloc(size, sizeof(int));
        
        if (rplm->modification_of_pic_nums_idc_l1 == NULL ||
            rplm->abs_diff_pic_num_minus1_l1 == NULL ||
            rplm->long_term_pic_num_l1 == NULL) {
            fprintf(stderr, "%s\n", "Alloc ref_pic_list_modification_l1 Error");
            exit(-1);
        }
    }else {
        rplm->modification_of_pic_nums_idc_l1 = NULL;
        rplm->abs_diff_pic_num_minus1_l1 = NULL;
        rplm->long_term_pic_num_l1 = NULL;
    }
}

/**
 解析pred_weight_table()句法元素
 [h264协议文档位置]：7.3.3.2 Prediction weight table syntax
 */
void parse_pred_weight_table(bs_t *b, sps_t *sps, slice_header_t *slice_header) {
    pred_weight_table_t *pw_table = &slice_header->pred_weight_table;
    
    pw_table->luma_log2_weight_denom = bs_read_ue(b);
    if (sps->chroma_format_idc != 0) {
        pw_table->chroma_log2_weight_denom = bs_read_ue(b);
    }
    
    for (int i = 0; i <= slice_header->num_ref_idx_l0_active_minus1; i++) {
        pw_table->luma_weight_l0_flag = bs_read_u(b, 1);
        
        // 参考7.4.3.2语义，对pw_table->luma_weight_l0_flag等于0时做如下修改
        if (pw_table->luma_weight_l0_flag) {
            pw_table->luma_weight_l0[i] = bs_read_se(b);
            pw_table->luma_offset_l0[i] = bs_read_se(b);
        }else {
            pw_table->luma_weight_l0[i] = 1 << pw_table->luma_log2_weight_denom;
            pw_table->luma_offset_l0[i] = 0;
        }
        
        if (sps->chroma_format_idc != 0) {
            pw_table->chroma_weight_l0_flag = bs_read_u(b, 1);
            // 参考7.4.3.2语义，对pw_table->chroma_weight_l0_flag等于0时做如下修改
            for (int j = 0; j < 2; j++) {
                if (pw_table->chroma_weight_l0_flag) {
                    pw_table->chroma_weight_l0[i][j] = bs_read_se(b);
                    pw_table->chroma_offset_l0[i][j] = bs_read_se(b);
                }else {
                    pw_table->chroma_weight_l0[i][j] = 1 << pw_table->chroma_log2_weight_denom;
                    pw_table->chroma_offset_l0[i][j] = 0;
                }
            }
        }
    }
    
    if (slice_header->slice_type == Slice_Type_B) {
        for (int i = 0; i <= slice_header->num_ref_idx_l1_active_minus1; i++) {
            pw_table->luma_weight_l1_flag = bs_read_u(b, 1);
            
            // 参考7.4.3.2语义，对pw_table->luma_weight_l1_flag等于0时做如下修改
            if (pw_table->luma_weight_l1_flag) {
                pw_table->luma_weight_l1[i] = bs_read_se(b);
                pw_table->luma_offset_l1[i] = bs_read_se(b);
            }else {
                pw_table->luma_weight_l1[i] = 1 << pw_table->luma_log2_weight_denom;
                pw_table->luma_offset_l1[i] = 0;
            }
            
            if (sps->chroma_format_idc != 0) {
                pw_table->chroma_weight_l1_flag = bs_read_u(b, 1);
                // 参考7.4.3.2语义，对pw_table->chroma_weight_l1_flag等于0时做如下修改
                for (int j = 0; j < 2; j++) {
                    if (pw_table->chroma_weight_l1_flag) {
                        pw_table->chroma_weight_l1[i][j] = bs_read_se(b);
                        pw_table->chroma_offset_l1[i][j] = bs_read_se(b);
                    }else {
                        pw_table->chroma_weight_l1[i][j] = 1 << pw_table->chroma_log2_weight_denom;
                        pw_table->chroma_offset_l1[i][j] = 0;
                    }
                }
            }
        }
    }
}

/**
 解析dec_ref_pic_marking()句法元素
 [h264协议文档位置]：7.3.3.3 Decoded reference picture marking syntax
 */
void parse_dec_ref_pic_marking(bs_t *b, slice_t *slice) {
    dec_ref_pic_marking_t *drp_marking = &slice->slice_header.dec_ref_pic_marking;
    int i, val;

    if (slice->idr_flag) {
        drp_marking->no_output_of_prior_pics_flag = bs_read_u(b, 1);
        drp_marking->long_term_reference_flag = bs_read_u(b, 1);
    }else {
        drp_marking->adaptive_ref_pic_marking_mode_flag = bs_read_u(b, 1);
        if (drp_marking->adaptive_ref_pic_marking_mode_flag) {
            i = 0;
            do {
                val = drp_marking->memory_management_control_operation[i] = bs_read_ue(b);
                if (val == 1 || val == 3) {
                    drp_marking->difference_of_pic_nums_minus1[i] = bs_read_ue(b);
                }
                if (val == 2) {
                    drp_marking->long_term_pic_num[i] = bs_read_ue(b);
                }
                if (val == 3 || val == 6) {
                    drp_marking->long_term_frame_idx[i] = bs_read_ue(b);
                }
                if (val == 4) {
                    drp_marking->max_long_term_frame_idx_plus1[i] = bs_read_ue(b);
                }
                i++;
            } while (val != 0);
        }
    }
}


/**
 计算CeilLog2(inputVal)
 */
unsigned calculateCeilLog2(unsigned inputVal)
{
    unsigned tmpVal = inputVal - 1;
    unsigned ret = 0;
    
    while( tmpVal != 0 ) {
        tmpVal >>= 1;
        ret++;
    }
    return ret;
}
