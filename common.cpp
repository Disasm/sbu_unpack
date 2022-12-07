#include "common.h"
#include <stdio.h>
#include <stdlib.h>

char* readString(const void* ptr, uint32_t* dataSize)
{
    uint16_t len = *(uint16_t*)ptr; 
    
    char* str = (char*)malloc(len*2 + 1);
    uint16_t* in = (uint16_t*)(((char*)ptr) + 2);
    char* p = str;
    for(size_t i=0; i<len; i++)
    {
        p = writeUtf(p, in[i]);
    }
    str[len] = 0;

    if(dataSize) *dataSize = (len + 1) *2;

    return str;
}

char* writeUtf(char* ptr, uint16_t ch)
{
    if(ch<0x7f)
    {
        *ptr = (char)ch;
        return ptr + 1;
    }
    else if(ch<0x7ff)
    {
        ptr[0] = (char)(0xC0 | (ch >> 6));
        ptr[1] = (char)(0x80 | (ch & 0x3F));
        return ptr + 2;
    }
    else
    {
        ptr[0] = (char)(0xE0 | (ch >> 12));
        ptr[1] = (char)(0x80 | ((ch >> 6) & 0x3F));
        ptr[2] = (char)(0x80 | (ch & 0x3F));
        return ptr + 3;
    }
}
