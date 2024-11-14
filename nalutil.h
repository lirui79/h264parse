#ifndef ___NALUTIL__H__
#define ___NALUTIL__H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t* start;
	uint8_t* p;
	uint8_t* end;
	int bits_left;
} bs_t;

#define _OPTIMIZE_BS_ 1

#if ( _OPTIMIZE_BS_ > 0 )
#ifndef FAST_U8
#define FAST_U8
#endif
#endif


bs_t* bs_new(uint8_t* buf, size_t size);

void bs_free(bs_t* b);

bs_t* bs_clone( bs_t* dest, const bs_t* src );

bs_t*  bs_init(bs_t* b, uint8_t* buf, size_t size);

uint32_t bs_byte_aligned(bs_t* b);

int bs_eof(bs_t* b);

int bs_overrun(bs_t* b);

int bs_pos(bs_t* b);

uint32_t bs_peek_u1(bs_t* b);


uint32_t bs_read_u1(bs_t* b);

uint32_t bs_read_u(bs_t* b, int n);

uint32_t bs_read_f(bs_t* b, int n);

uint32_t bs_read_u8(bs_t* b);

uint32_t bs_read_ue(bs_t* b);

int32_t  bs_read_se(bs_t* b);


void bs_write_u1(bs_t* b, uint32_t v);

void bs_write_u(bs_t* b, int n, uint32_t v);

void bs_write_f(bs_t* b, int n, uint32_t v);

void bs_write_u8(bs_t* b, uint32_t v);

void bs_write_ue(bs_t* b, uint32_t v);

void bs_write_se(bs_t* b, int32_t v);


int bs_read_bytes(bs_t* b, uint8_t* buf, int len);

int bs_write_bytes(bs_t* b, uint8_t* buf, int len);

int bs_skip_bytes(bs_t* b, int len);

uint32_t bs_next_bits(bs_t* b, int nbits);

uint64_t bs_next_bytes(bs_t* bs, int nbytes);

#ifdef __cplusplus
}
#endif


#endif /* ___NALUTIL__H__ */
