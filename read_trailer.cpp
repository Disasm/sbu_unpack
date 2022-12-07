#include <stdio.h>
#include <stdlib.h>
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

// TODO add dirname
void dumpFile(FILE *fsbu, long offset, size_t size, char *filename)
{
	char outputpath[1000];
	sprintf(outputpath, "trailer/%s", filename);
	FILE *f = fopen(outputpath, "w");
	if(!f)
	{
		fprintf(stderr, "Can't open file '%s'\n", outputpath);
		return;
	}

	fseek(fsbu, offset, SEEK_SET);
	void *data = malloc(size);
	fread(data, 1, size, fsbu);
	fwrite(data, size, 1, f);
	fclose(f);
}


int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s %s <file.sbu> <trailerfile.tr>\n", argv[0], argv[1]);
		return 1;
	}

	FILE* fsbu = fopen(argv[1], "r");
	if(!fsbu)
	{
		fprintf(stderr, "Can't open file '%s'\n", argv[1]);
		return 1;
	}

	FILE* ft = fopen(argv[2], "r");
	if(!ft)
	{
		fprintf(stderr, "Can't open file '%s'\n", argv[2]);
		return 1;
	}

	fseek(ft, 0, SEEK_END);
	long fsize = ftell(ft);
	fseek(ft, 0, SEEK_SET);  /* same as rewind(f); */

	char *ptr = (char *)malloc(fsize + 1);
	fread(ptr, fsize, 1, ft);

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
		dumpFile(fsbu, de1->offset, de2->size, str2);
	}


	fclose(fsbu);
	fclose(ft);
}
