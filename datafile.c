#include "datafile.h"

unsigned long getfilesize(const char *filename) {
    unsigned long filesize = 0;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    fpos_t fpos; //当前位置
    fgetpos(fp, &fpos); //获取当前位置
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fsetpos(fp,&fpos); //恢复之前的位置
    fclose(fp);
    return filesize;
}

unsigned long readfilebuffer(const char *filename, unsigned char *buffer, unsigned long bufsize) {
    unsigned long readsize = 0;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    readsize = fread(buffer, 1, bufsize, fp);
    fclose(fp);
    return readsize;
}
