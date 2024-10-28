#include "stream.h"

unsigned long getStreamfileSize(const char *filename) {
    unsigned long fileSize = 0;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    fpos_t fpos; //当前位置
    fgetpos(fp, &fpos); //获取当前位置
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fsetpos(fp,&fpos); //恢复之前的位置
    fclose(fp);
    return fileSize;
}

unsigned long readStreambuffer(const char *filename, unsigned char *buffer, unsigned long bufSize) {
    unsigned long readSize = 0;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    readSize = fread(buffer, 1, bufSize, fp);
    fclose(fp);
    return readSize;
}
