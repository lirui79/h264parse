
#include "slice.h"
#include <stdlib.h>

void processSlice(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice) {
    // 0.先解析slice_header
    processSliceHeader(b, sps, pps, slice);
}

// 初始化slice结构体
slice_t *allocSlice(void)
{
    slice_t *slice = (slice_t*) malloc(sizeof(slice_t));
    if (slice == NULL) {
        fprintf(stderr, "%s\n", "Alloc PPS Error");
        return NULL;
    }
    memset(slice, 0, sizeof(slice_t));
    return slice;
}

// 释放slice
void freeSlice(slice_t *slice) {
   if (slice != NULL)
      free(slice);
}
