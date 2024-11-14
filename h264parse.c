
#include <stdio.h>
#include "nalu.h"
#include "datafile.h"
#include "avc_stream.h"
#include "slice.h"

int main(int argc, const char * argv[]) {
    if(argc < 2) {
        printf("please input: ./%s **.h264\n", argv[0]);
        return -1;
    }
    // 0. 读取h264文件
    unsigned int bufsize = getfilesize(argv[1]);
    uint8_t *buffer = (uint8_t*) malloc(bufsize);
    printf("The file %s totalSize:%d\n", argv[1], bufsize);
    bufsize = readfilebuffer(argv[1], buffer, bufsize);
    
    // 1. 开辟nalu_t保存nalu_header和SODB
    nalu_t *nalu = alloc_nalu();
    
    slice_t *slice = allocSlice();
    sps_t   *sps   = allocSPS();
    pps_t   *pps   = allocPPS();

    int nalu_i = 0;
    int startpos = 0;  // 当前找到的nalu起始位置
    
    // 2.找到h264码流中的各个nalu
    while (1) {
        startpos = find_nalu(buffer, bufsize, startpos, nalu);
        printf("nalu: %d, start: %d, size: %d\n", nalu_i, startpos, nalu->len);
        if (nalu->len <= 0)
            break;
        
        // 读取/解析 nalu
        parse_nalu(nalu, sps, pps, slice);
        
        ++nalu_i;
        startpos += nalu->len;
    }
    
    freeSPS(sps);
    freePPS(pps);
    freeSlice(slice);
    free_nalu(nalu);
   
    return 0;
}
