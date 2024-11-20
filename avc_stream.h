
#ifndef ___AVC_STREAM__H__
#define ___AVC_STREAM__H__


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "nalu.h"
#include "nalutil.h"
#include "avc_sps.h"
#include "avc_pps.h"
#include "avc_slice.h"

#ifdef __cplusplus
extern "C" {
#endif

int  parse_sps(bs_t *b, sps_t *sps);

int  parse_pps(bs_t *b, sps_t *sps, pps_t *pps);


void scaling_list(bs_t *b, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag);


void parse_slice(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice);

void parse_slice_header(bs_t *b, sps_t *sps, pps_t *pps, slice_t *slice);

unsigned int parse_nalu(nalu_t *nalu, sps_t *sps, pps_t *pps, slice_t *slice);

#ifdef __cplusplus
}
#endif

#endif /* ___AVC_STREAM__H__ */
