
#ifndef ___DATA_FILE__H__
#define ___DATA_FILE__H__


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

unsigned long getfilesize(const char *filename);

unsigned long readfilebuffer(const char *filename, unsigned char *buffer, unsigned long bufsize);

#ifdef __cplusplus
}
#endif

#endif /* ___DATA_FILE__H__ */
