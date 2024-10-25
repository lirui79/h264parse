
#ifndef ___STREAM_FILE__H__
#define ___STREAM_FILE__H__


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

unsigned long getStreamfileSize(const char *filename);

unsigned long readStreambuffer(const char *filename, unsigned char *buffer, unsigned long bufSize);

#ifdef __cplusplus
}
#endif

#endif /* ___STREAM_FILE__H__ */
