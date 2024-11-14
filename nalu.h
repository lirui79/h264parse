
#ifndef __NALU_H_
#define __NALU_H_

#include <stdio.h>
#include <stdlib.h>
#include "nalutil.h"
#include "slice.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************************************
**                                        nalu header: 负责将VCL产生的比特字符串适配到各种各样的网络和多元环境中,
                                                       覆盖了所有片级以上的语法级别(NALU的作用, 方便网络传输)
**
-------------------------------------------------------------------------------------------------------------
**        字段名称            　　 |    长度(bits)    |        有关描述
-------------------------------------------------------------------------------------------------------------
**        forbidden_bit          |    1             |        编码中默认值为0, 当网络识别此单元中存在比特错误时, 可将其设为1, 以便接收方丢掉该单元
**        nal_reference_idc      |    2             |        0~3标识这个NALU的重要级别
**        nal_unit_type          |    5             |          NALU的类型(类型1~12是H.264定义的, 类型24~31是用于H.264以外的,
                                                             RTP负荷规范使用这其中的一些值来定义包聚合和分裂, 其他值为H.264保留)

** nal_unit_type:
    0                未使用
    1                未使用Data Partitioning, 非IDR图像的Slice
    2                使用Data Partitioning且为Slice A
    3                使用Data Partitioning且为Slice B
    4                使用Data Partitioning且为Slice C
    5                IDR图像的Slice(立即刷新)
    6                补充增强信息(SEI)
    7                序列参数集(sequence parameter set, SPS)
    8                图像参数集(picture parameter set, PPS)
    9                分界符
    10                序列结束
    11                码流结束
    12                填充
    13...23            保留
    24...31            未使用

** SPS, PPS. SLICE等信息就不解析了. 为了减少bits, 用了哥伦布编码(自己解析比较麻烦, 但是网上有很多).

** SPS信息说明:
        1. 视频宽高, 帧率等信息;
        2. seq_parameter_set_id, 指明本序列参数集的id号, 这个id号将被picture参数集引用;
        3. pic_width_in_mbs_minus1, 加1指定以宏块(16*16)为单位的每个解码图像的宽度, 即width = (pic_width_in_mbs_minus1 + 1) * 16
        4. pic_height_in_map_units_minus1;
        5. pic_order_cnt_type, 视频的播放顺序序号叫做POC(picture order count), 取值0,1,2;
        6. time_scale, fixed_frame_rate_flag, 计算帧率(fps).
           视频帧率信息在SPS的VUI parameters syntax中, 需要根据time_scale, fixed_frame_rate_flag计算得到: fps = time_scale / num_units_in_tick.
           但是需要判断参数timing_info_present_flag是否存在, 若不存在表示FPS在信息流中无法获取.
           同时还存在另外一种情况: fixed_frame_rate_flag为1时, 两个连续图像的HDR输出时间频率为单位, 获取的fps是实际的2倍.

** PPS信息说明:
        1. pic_parameter_set_id, 用以指定本参数集的序号, 该序号在各片的片头被引用;
        2. seq_parameter_set_id, 指明本图像参数集所引用的序列参数集的序号;
        3. 其他高深的暂时还不理解, 指明参考帧队列等.

** SLICE信息说明:
        1. slice_type, 片的类型;
        2. pic_parameter_set_id, 引用的图像索引;
        3. frame_num, 每个参考帧都有一个连续的frame_num作为它们的标识, 它指明了各图像的解码顺序. 非参考帧也有，但没有意义;
        4. least significant bits;
        5. 综合三种poc(pic_order_cnt_type), 类型2应该是最省bit的, 因为直接从frame_num获得, 但是序列方式限制最大;
           类型1, 只需要一定的bit量在sps标志出一些信息还在slice header中表示poc的变化, 但是比类型0要节省bit, 但是其序列并不是随意的, 要周期变化;
           对于类型0因为要对poc的lsb(pic_order_cnt_lsb, last bit)进行编码所以用到的bit最多, 优点是序列可以随意.
           ** 自我理解, 不一定准确(这边算显示顺序, 要根据SPS中的pic_order_cnt_type, 为2, 意味着码流中没有B帧, frame_num即为显示顺序;
              为1, 依赖frame_num求解POC; 为0, 把POC的低位编进码流内, 但这只是低位, 而POC的高位PicOrderCntMsb则要求解码器自行计数,
              计数方式依赖于前一编码帧(PrevPicOrderCntMsb与PrevPicOrderCntLsb.

           ** 一般的码流分析所见(未仔细证实): pic_order_cnt_type=2, 只有frame_num(无B帧);
              pic_order_cnt_type=1, 暂未分析到;
              pic_order_cnt_type=0, pic_order_cnt_lsb指示显示顺序, 一般为偶数增长(0, 2, 4, 6, 据说是什么场方式和帧方式, 场时其实是0 0 2 2 4 4).

           ** 编码与显示的原因: 视频编码顺序与视频的播放顺序, 并不完全相同, 视频编码时, 如果采用了B帧编码, 由于B帧很多时候都是双向预测得来的,
              这时会先编码B帧的后向预测图像(P帧), 然后再进行B帧编码, 因此会把视频原来的播放顺序打乱, 以新的编码顺序输出码流,
              而在解码断接收到码流后, 需要把顺序还原成原本的播放顺序, 以输出正确的视频. 在编解码中, 视频的播放顺序序号叫做POC(picture order count).

** 总结: 1. 码流中有很多SPS(序列), 一个序列中有多个图像, 一个图像中有多个片, 一个片中有多个块;
         2. SPS中有seq_parameter_set_id. PPS中有pic_parameter_set_id, 并通过seq_parameter_set_id指明关联的序列.
            SLICE中有pic_parameter_set_id, 指明关联的图像;
         3. SPS中可计算宽高以及帧率, pic_order_cnt_type(显示顺序的类型);
            SLICE HEADER中可算出解码的顺序, 以及根据pic_order_cnt_type算出显示顺序.
************************************************************************************************************/

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
    H264_NAL_UNSPECIFIED     = 0,
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
    H264_NAL_PREFIX          = 14,
    H264_NAL_SUB_SPS         = 15,
    H264_NAL_DPS             = 16,
    H264_NAL_RESERVED17      = 17,
    H264_NAL_RESERVED18      = 18,
    H264_NAL_AUXILIARY_SLICE = 19,
    H264_NAL_EXTEN_SLICE     = 20,
    H264_NAL_DEPTH_EXTEN_SLICE = 21,
    H264_NAL_RESERVED22      = 22,
    H264_NAL_RESERVED23      = 23,
    H264_NAL_UNSPECIFIED24   = 24,
    H264_NAL_UNSPECIFIED25   = 25,
    H264_NAL_UNSPECIFIED26   = 26,
    H264_NAL_UNSPECIFIED27   = 27,
    H264_NAL_UNSPECIFIED28   = 28,
    H264_NAL_UNSPECIFIED29   = 29,
    H264_NAL_UNSPECIFIED30   = 30,
    H264_NAL_UNSPECIFIED31   = 31,
};

/* 1. check h264 startcode: 0x000001(3 Bytes)或0x00000001(4 Bytes);
 * 2. bitstream annexb use startcode divide NALU.
 * rawbs -  input data pointer
 * return  0x00  - no startcode ;  0x03 - startcode 0x000001(3 Bytes) ; 0x04 - startcode 0x00000001(4 Bytes)
 */
unsigned int check_startcode(uint8_t *rawbs);

/* find h264 startcode position
 * rawbs     -  input data pointer
 * size      -  input data size
 * startpos  -  input data offset
 * return   startpos  startcode pos
 */
unsigned int find_startcode(uint8_t *rawbs, unsigned int size, unsigned int startpos);

/* find raw h264 bs data nal data size, not include startcode
 * rawbs -  input h264 bs data pointer
 * size  -  input h264 bs data size
 * startpos - find nal data start position
 * nalu  -  find nalu set buf and len
 * return  startpos
 */
unsigned int find_nalu(uint8_t *rawbs, unsigned int size, unsigned int startpos, nalu_t *nalu);

unsigned int parse_nalu(nalu_t *nalu, sps_t *sps, pps_t *pps, slice_t *slice);

/**
 去除rbsp中的0x03
 @see 7.3.1 NAL unit syntax
 @see 7.4.1.1 Encapsulation of an SODB within an RBSP
 @return 返回去除0x03后nalu的大小
 */
unsigned int emulation_prevention_three_byte(const uint8_t *rawbs, unsigned int size, uint8_t* rbbuf, unsigned int* rbsize);

/**
 计算SODB的长度
 【注】RBSP = SODB + trailing_bits
 */
unsigned int rbsp_trailing_bits_size(nalu_t *nalu);

// 在rbsp_trailing_bits()之前是否有更多数据
unsigned int more_rbsp_data(bs_t *b);

nalu_t *alloc_nalu();

void free_nalu(nalu_t *nalu);

#ifdef __cplusplus
}
#endif

#endif /* nalu_h */
