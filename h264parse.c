
#include <stdio.h>
#include "nalu.h"
#include "stream.h"
#include "slice.h"

int main(int argc, const char * argv[]) {
    if(argc < 2) {
        printf("please input: ./%s **.h264\n", argv[0]);
        return -1;
    }
    // 0. 读取h264文件
    unsigned int bufSize = getStreamfileSize(argv[1]);
    uint8_t *buffer = (uint8_t*) malloc(bufSize);
    printf("The file %s totalSize:%d\n", argv[1], bufSize);
    bufSize = readStreambuffer(argv[1], buffer, bufSize);
    
    // 1. 开辟nalu_t保存nalu_header和SODB
    nalu_t *nalu = allocNalu();
    
    slice_t *slice = allocSlice();
    sps_t   *sps   = allocSPS();
    pps_t   *pps   = allocPPS();

    int nalu_i = 0;
    int startPos = 0;  // 当前找到的nalu起始位置
    
    // 2.找到h264码流中的各个nalu
    while (1) {
        startPos = FindNaluPos(buffer, bufSize, startPos, nalu);
        printf("nalu: %d, start: %d, size: %d\n", nalu_i, startPos, nalu->len);
        if (nalu->len <= 0)
            break;
        
        // 读取/解析 nalu
        ParseNalu(nalu, sps, pps, slice);
        
        ++nalu_i;
        startPos += nalu->len;
    }
    
    freeSPS(sps);
    freePPS(pps);
    freeSlice(slice);
    freeNalu(nalu);
   
    return 0;
}
