#include "filesystem.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

const char *FileSystem::fileExtName(const char *filename)
{
    if (filename == nullptr)
        return nullptr;

    size_t len = strlen(filename);
    for (int i = len - 1; i >= 0; i--) {
        if (filename[i] == '.' && i + 1 < (int)len)
            return (filename + i + 1);
    }

    return nullptr;
}

char *FileSystem::readAll(const char *filename)
{
    if (filename == nullptr)
        return nullptr;

    FILE *fp = fopen(filename, "r");
    if (fp == nullptr)
        return nullptr;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = new char[size + 1];

#ifdef _WIN32
	size_t idx = 0;
#else
	fpos_t pos;

	pos.__pos = 0;
#endif

    while (!feof(fp))
	{
#ifdef _WIN32
		fgets(buffer + idx, size, fp);
		idx = strlen(buffer);
#else
       fgets((buffer + pos.__pos), size, fp);
	   fgetpos(fp, &pos);
#endif
    }

    fclose(fp);
    return buffer;
}
