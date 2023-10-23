#ifndef VERTEX_BUFFEROBJECT_H
#define VERTEX_BUFFEROBJECT_H

#include "common.h"

class BufferObject
{
public:
    BufferObject();
    BufferObject(GLenum type);
    ~BufferObject();

    void allocate(const void *data, int count);

    void create();
    void bind();

    bool isCreated() const { return mBufferObjectId != 0; }

    void release();
    void destroy();

    void setUsagePattern(GLenum pattern) { mPattern = pattern; }

    GLuint bufferId() const { return mBufferObjectId; }
    GLenum type() const { return mType; }
    GLenum usagePattern() const { return mPattern; }

private:
    GLenum mType;
    GLenum mPattern;
    GLuint mBufferObjectId;
};

#endif // BUFFEROBJECT_H
