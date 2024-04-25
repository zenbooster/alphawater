#ifndef FILESYSTEM_H
#define FILESYSTEM_H

class FileSystem
{
public:
    static const char *fileExtName(const char *filename);
    static char *readAll(const char *filename);
};

#endif // FILESYSTEM_H
