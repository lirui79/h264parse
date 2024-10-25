//
//  nalu.c
//  H264Analysis
//
//  Created by Jinmmer on 2018/5/16.
//  Copyright © 2018年 Jinmmer. All rights reserved.
//

#include "nalu.h"
#include <assert.h>
#include "stream.h"
#include "parset.h"
#include "slice.h"

extern slice_t *currentSlice;

/* 1. h264的起始码: 0x000001(3 Bytes)或0x00000001(4 Bytes);
 * 2. 文件流中用起始码来区分NALU.
 * nal -  input data pointer
 * return  0x00  - no find startcode   0x03 - startcode 3 bytes  0x04 - startcode 4 bytes
 */ 
unsigned int FindStartCode(uint8_t *rawbs) {
    if ((0x00 != rawbs[0]) || (0x00 != rawbs[1])) {
        return 0x00;
    }

    if (0x01 == rawbs[2]) {
        return 0x03;
    }

    if ((0x00 != rawbs[2]) || (0x01 != rawbs[3])) {
        return 0x00;
    }
    return 0x04;
}

unsigned  int FindNaluPos(uint8_t *rawbs, unsigned int size, unsigned int startPos) {
    while (startPos < size) {
        if (FindStartCode(rawbs + startPos) > 0) {
            break;
        } else {
            ++startPos;
        }
    }
    return startPos;
}

/* find raw h264 bs data nal data size, not include startcode
 * 
 * rawbs -  input h264 bs data pointer
 * size  -  input h264 bs data size
 * startPos - find nal data start position
 * nalu  -  find nalu set buf and len
 * return  startPos
 */ 
unsigned int FindNalu(uint8_t *rawbs, unsigned int size, unsigned int startPos, nalu_t *nalu) {
    unsigned int endPos = 0, startCode = 3;
    while (startPos < size) {
        startCode = FindStartCode(rawbs + startPos);
        if (startCode > 0) {
            break;
        } else {
            ++startPos;
        }
    }

    endPos = startPos + 3;
    while (endPos < size) {
        if (FindStartCode(rawbs + endPos) > 0) {
            break;
        } else {
            ++endPos;
        }
    }

    nalu->buf = rawbs + startPos + startCode;
    nalu->len = (endPos - startPos - startCode);
    return (startPos + startCode); // if file is end
}

/**
 去除rbsp中的0x03
 @see 7.3.1 NAL unit syntax
 @see 7.4.1.1 Encapsulation of an SODB within an RBSP
 @return 返回去除0x03后nalu的大小
 */
unsigned int ConverNaluToRbsp(const uint8_t *rawbs, unsigned int size, uint8_t* rbbuf, unsigned int* rbsize) {
    // 遇到0x000003则把03去掉，包含以cabac_zero_word结尾时，尾部为0x000003的情况
    unsigned int i = 0, j = 0;
    for (i = 0; i < size; ) {
        rbbuf[j] = rawbs[i];
        if ((0x00 != rawbs[i]) || (i >= (size - 3)))  {
            ++i;
            ++j;
            continue;
        }

        rbbuf[j + 1] = rawbs[i + 1];
        if (0x00 != rawbs[i + 1]) {
            i += 2;         
            j += 2;   
            continue;
        }

        if (rawbs[i + 2] == 0x03) {
            i += 3;
            j += 2;
            continue;
        }
        rbbuf[j + 2] = rawbs[i + 2];
        i += 3;
        j += 3;
    }

    *rbsize = j;
    return j;
}

/**
 计算SODB的长度
 【注】RBSP = SODB + trailing_bits
 */
unsigned int ConverRbspToSodb(nalu_t *nalu)
{
    int ctr_bit, bitoffset, last_byte_pos;
    bitoffset = 0;
    last_byte_pos = nalu->len - 1;
    
    // 0.从nalu->buf的最末尾的比特开始寻找
    ctr_bit = (nalu->buf[last_byte_pos] & (0x01 << bitoffset));
    
    // 1.循环找到trailing_bits中的rbsp_stop_one_bit
    while (ctr_bit == 0) {
        ++bitoffset;
        if(bitoffset == 8) {
            // 因nalu->buf中保存的是nalu_header+RBSP，因此找到最后1字节的nalu_header就宣告RBSP查找结束
            if(last_byte_pos == 1)
                printf(" Panic: All zero data sequence in RBSP \n");
            assert(last_byte_pos != 1);
            last_byte_pos -= 1;
            bitoffset = 0;
        }
        ctr_bit= nalu->buf[last_byte_pos-1] & (0x01 << bitoffset);
    }
    // 【注】函数开始已对last_byte_pos做减1处理，此时last_byte_pos表示相对于SODB的位置，然后赋值给nalu->len得到最终SODB的大小
    return last_byte_pos;
}

/**
 在rbsp_trailing_bits()之前是否有更多数据
 [h264协议文档位置]：7.2 Specification of syntax functions, categories, and descriptors
 */
unsigned int MoreRbspData(bs_t *b) 
{
    // 0.是否已读到末尾
    if (bs_eof(b)) {
        return 0;
    }
    
    // 1.下一比特值是否为0，为0说明还有更多数据
    if (bs_peek_u1(b) == 0) {
        return 1;
    }
    
    // 2.到这说明下一比特值为1，这就要看看是否这个1就是rbsp_stop_one_bit，也即1后面是否全为0
    bs_t bs_temp;
    bs_clone(&bs_temp, b);
    
    // 3.跳过刚才读取的这个1，逐比特查看1后面的所有比特，直到遇到另一个1或读到结束为止
    bs_read_u1(&bs_temp);
    while(!bs_eof(&bs_temp)) {
        if (bs_read_u1(&bs_temp) == 1) { 
            return 1; 
        }
    }
    
    return 0;
}

/**
 读取一个nalu
 @see 7.3.1 NAL unit syntax
 @see 7.4.1 NAL unit semantics
 */
unsigned int ParseNalu(nalu_t *nalu) {
    uint8_t* buffer = NULL;
    unsigned int bufSize = nalu->len;
    buffer = malloc(bufSize);
    // 1.去除nalu中的emulation_prevention_three_byte：0x03
    ConverNaluToRbsp(nalu->buf, nalu->len, buffer, &bufSize);
    nalu->buf = buffer;
    nalu->len = bufSize;
    // 2.初始化逐比特读取工具句柄
    bs_t *bs = bs_new(buffer, bufSize);
    
    // 3. 读取nal header 7.3.1
    nalu->forbidden_zero_bit = bs_read_u(bs, 1);
    nalu->nal_ref_idc = bs_read_u(bs, 2);
    nalu->nal_unit_type = bs_read_u(bs, 5);
    
    printf("\tnal->forbidden_zero_bit: %d\n", nalu->forbidden_zero_bit);
    printf("\tnal->nal_ref_idc: %d\n", nalu->nal_ref_idc);
    printf("\tnal->nal_unit_type: %d\n", nalu->nal_unit_type);

    switch (nalu->nal_unit_type)
    {
        case H264_NAL_SPS:
            nalu->len = ConverRbspToSodb(nalu);
            processSPS(bs);
            break;
            
        case H264_NAL_PPS:
            nalu->len = ConverRbspToSodb(nalu);
            processPPS(bs);
            break;
            
        case H264_NAL_SLICE:
        case H264_NAL_IDR_SLICE:
            currentSlice->idr_flag = (nalu->nal_unit_type == H264_NAL_IDR_SLICE);
            currentSlice->nal_ref_idc = nalu->nal_ref_idc;
            nalu->len = ConverRbspToSodb(nalu);
            processSlice(bs);
            break;
            
        case H264_NAL_DPA:
            nalu->len = ConverRbspToSodb(nalu);
            break;
            
        case H264_NAL_DPB:
            nalu->len = ConverRbspToSodb(nalu);
            break;
            
        case H264_NAL_DPC:
            nalu->len = ConverRbspToSodb(nalu);
            break;
            
        default:
            break;
    }
    free(buffer);
    bs_free(bs);
}



nalu_t *allocNalu() {
    // calloc：初始化所分配的内存空间
    nalu_t *nalu = (nalu_t *)malloc(sizeof(nalu_t));
    if (nalu == NULL) {
        fprintf(stderr, "%s\n", "AllocNALU: nalu");
        return NULL;
    }
    memset(nalu, 0, sizeof(nalu_t));
    return nalu;
}

void freeNalu(nalu_t *nalu) {
    free(nalu);
}
