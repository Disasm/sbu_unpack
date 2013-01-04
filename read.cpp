#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <list>

typedef struct
{
    uint8_t uid[16];
    uint16_t type;
    uint32_t headerSize;
    uint64_t dataSize;
    uint64_t trailerSize;
    uint64_t dataSize2;
    uint64_t trailerSize2;
} __attribute__ ((packed)) sbu_file_header_t;

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

typedef struct
{
    uint32_t offset;
    uint32_t size;
} file_t;

const char* getNameByUid(const char* uid)
{
    if(strcmp(uid, "d80e93dbb13849c891a350e78921b742")==0) return ""; // image?
    if(strcmp(uid, "b8175a8c864c45069b500cc88828ee2e")==0) return "Phonebook.bk";
    if(strcmp(uid, "ed258ddeff894c4386fabcae9093195e")==0) return "Calendar.bk";
    if(strcmp(uid, "dc23289f69ad447d83ac992ee77777da")==0) return "Memo.bk";
    if(strcmp(uid, "0bfffeb46d4b468d833250877abe5675")==0) return "Configuration.bk";
    if(strcmp(uid, "c77c44b87fad4a748731dca0c327b6ff")==0) return "Email.bk";
    if(strcmp(uid, "dcddf476763141ef85b0d8a156abedff")==0) return "Group.bk";
    if(strcmp(uid, "31dc09408e014cb491c4166801f83f17")==0) return "Task.bk";
    if(strcmp(uid, "049ba2d7ea674a23bae4fd93101fd9e0")==0) return "Message.bk";
    if(strcmp(uid, "42f412b8aca449f18e928823e318034d")==0) return "Network.bk";
    if(strcmp(uid, "663c6e54e2a34a8dbe56b4cce1a99303")==0) return "Logs.bk";
    
    // directories
    if(strcmp(uid, "48056f2310864f53bb46468d6479a810")==0) return "Photo";
    if(strcmp(uid, "f448c42b268b4ba28363735e41de2e10")==0) return "Applications";
    if(strcmp(uid, "6967fd0411274d2491246ac48c5dde1a")==0) return "Other";
    if(strcmp(uid, "0e0fe262f75b4b079046ba7cb7e86a19")==0) return "DRM";
    if(strcmp(uid, "ae4fee74354440d4b6c5fb220030458a")==0) return "Navigation";
    if(strcmp(uid, "dd3f966286cf4378af6e72c7dbf5575f")==0) return "JAVA";
    fprintf(stderr, "Unknown UID: %s\n", uid);
    return NULL;
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

char* readString(FILE* f)
{
    uint32_t pos = ftell(f);

    uint16_t len;
    if(fread(&len, sizeof(len), 1, f) != 1) return 0;

    char* str = (char*)malloc(len*2 + 1);
    char* ptr = str;

    for(size_t i=0; i<len; i++)
    {
        uint16_t buf;
        if(fread(&buf, sizeof(buf), 1, f) != 1)
        {
            free(str);
            fprintf(stderr, "Error reading string at 0x%x\n", pos);
            return 0;
        }
        ptr = writeUtf(ptr, buf);
    }
    str[len] = 0;

    return str;
}

void dumpTrailer(char* ptr)
{
    printf("trailer:\n");

    uint32_t count = *(uint32_t*)ptr;
    printf("  count: 0x%x\n", count);

    ptr += 4;

    for(uint32_t i=0; i<count; i++)
    {
        sbu_dirent_t* de1 = (sbu_dirent_t*)ptr;
        ptr += sizeof(sbu_dirent_t);

        uint32_t sz;
        char* str1 = readString(ptr, &sz);
        ptr += sz;
        char* str2 = readString(ptr, &sz);
        ptr += sz;

        sbu_dirent2_t* de2 = (sbu_dirent2_t*)ptr;
        ptr += sizeof(sbu_dirent2_t);

        printf("  file offset=0x%08x size=0x%08x '%s' '%s'\n", de1->offset, de2->size, str1, str2);
    }
}

bool unpackFile(FILE* f, uint32_t offset, uint32_t size)
{
    printf("========= unpacking file at 0x%08x ==========\n", offset);
    fseek(f, offset, SEEK_SET);

    sbu_file_header_t header;

    if(fread(&header, sizeof(header), 1, f) != 1) return false;
    size -= sizeof(header);

    char uid[33];
    for(int i=0; i<16; i++)
    {
        sprintf(uid + i*2, "%02x", header.uid[i]);
    }

    printf("uid: %s (%s)\n", uid, getNameByUid(uid));
    printf("type:          0x%x\n", (uint32_t)header.type);
    printf("header size:   0x%x\n", (uint32_t)header.headerSize);
    printf("data size:     0x%x\n", (uint32_t)header.dataSize);
    printf("data size2:    0x%x\n", (uint32_t)header.dataSize2);
    printf("trailer size:  0x%x\n", (uint32_t)header.trailerSize);
    printf("trailer size2: 0x%x\n", (uint32_t)header.trailerSize2);

    char fileName[50];
    sprintf(fileName, "out/%08x_%s", offset, uid);

    // read data
    void* data = malloc(header.dataSize);
    fread(data, 1, header.dataSize, f);

    FILE* f2 = fopen(fileName, "w");
    fwrite(data, 1, header.dataSize, f2);
    fclose(f2);
    
    free(data);

    if(header.type==0)
    {
        // read trailer
        data = malloc(header.trailerSize);
        fread(data, 1, header.trailerSize, f);
    
        strcat(fileName, ".tr");
        f2 = fopen(fileName, "w");
        fwrite(data, 1, header.trailerSize, f2);
        fclose(f2);

        dumpTrailer((char*)data);
    
        free(data);
    }
    else if(header.type==2)
    {
        printf("trailer str1: '%s'\n", readString(f));
        printf("trailer str2: '%s'\n", readString(f));
    }
    
    return true;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <file.sbu>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if(!f)
    {
        fprintf(stderr, "Can't open file '%s'\n", argv[1]);
        return 1;
    }

    char buf[100];
    fread(buf, 1, 4, f);

    if(strncmp(buf, "SBU_", 4) != 0)
    {
        fprintf(stderr, "Incorrect header\n");
        fclose(f);
        return 1;
    }

    uint16_t num;
    fread(&num, 2, 1, f);
    printf("num1=0x%04x\n", num);
    fread(&num, 2, 1, f);
    printf("num2=0x%04x\n", num);
    fread(&num, 2, 1, f);
    printf("num3=0x%04x\n", num);

    char* str;
    str = readString(f);
    printf("str1='%s'\n", str);
    str = readString(f);
    printf("str2='%s'\n", str);
    str = readString(f);
    printf("str3='%s'\n", str);
    str = readString(f);
    printf("str4='%s'\n", str);
    str = readString(f);
    printf("str5='%s'\n", str);

    uint32_t numEntries;
    fread(&numEntries, 4, 1, f);
    printf("numEntries=0x%x\n", numEntries);

    std::list<file_t> files;
    for(uint32_t k=0; k<numEntries; k++)
    {
        uint8_t uid[16];
        uint64_t offset;
        uint64_t size;

        fread(uid, 1, 16, f);
        fread(&offset, 8, 1, f);
        fread(&size, 8, 1, f);
        fseek(f, 6, SEEK_CUR); // skip 6 bytes

        if(offset==0) continue;
        
        printf("%02x [uid:", k);
        for(int i=0; i<16; i++)
        {
            printf("%02x", (uint8_t)uid[i]);
        }
        printf(" offset: 0x%08x size: 0x%08x]\n", (uint32_t)offset, (uint32_t)size);

        file_t f1;
        f1.offset = offset;
        f1.size = size;
        files.push_back(f1);
    }

    for(std::list<file_t>::const_iterator it=files.begin(); it!=files.end(); it++)
    {
        const file_t& file = *it;
        unpackFile(f, file.offset, file.size);
    }

    fclose(f);

    return 0;
}

