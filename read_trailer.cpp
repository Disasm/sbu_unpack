#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"

void dumpFile(FILE *fsbu, long offset, size_t size, char *dirname, char *filename)
{
	char *p;
	if((p=index(dirname, ':')))
	{
		dirname = p;
		dirname++;
	}

	if(*dirname=='\\')
	{
		dirname++;
	}

	p = dirname;
	while(*p!='\0')
	{
		if(*p=='\\')
		{
			*p='_';
		}
		p++;
	}


	char outputpath[1000];
	sprintf(outputpath, "trailer/%s%s", dirname, filename);
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

void handleTrailer(char *trailerFilePath, FILE *fsbu)
{
	FILE* ft = fopen(trailerFilePath, "r");
	if(!ft)
	{
		fprintf(stderr, "Can't open file '%s'\n", trailerFilePath);
		return;
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
		dumpFile(fsbu, de1->offset, de2->size, str1, str2);
	}

	fclose(ft);
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s <file.sbu> <trailerfile.tr> [<trailerfile2.tr>] [...]\n", argv[0]);
		return 1;
	}

	FILE* fsbu = fopen(argv[1], "r");
	if(!fsbu)
	{
		fprintf(stderr, "Can't open file '%s'\n", argv[1]);
		return 1;
	}

	for(int i=2; i<argc; i++)
	{
		handleTrailer(argv[i], fsbu);
	}

	fclose(fsbu);
}
