#include <stdint.h>

typedef struct
{   
    uint16_t zero1;
    uint32_t offset;
    uint32_t zero2;
    // name1, name2
} __attribute__ ((packed)) sbu_dirent_t;

typedef struct
{
    // name1, name2
    uint32_t zero1;
    uint32_t size;
    uint32_t unk1; // checksum?
    uint32_t zero2;
    uint32_t unk2; //0x24, 0x27
} __attribute__ ((packed)) sbu_dirent2_t;

char* readString(const void* ptr, uint32_t* dataSize);
char* writeUtf(char* ptr, uint16_t ch);
