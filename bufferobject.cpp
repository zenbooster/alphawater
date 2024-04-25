#include "bufferobject.h"
#include "common.h"

BufferObject::BufferObject()
    : BufferObject(GL_VERTEX_ARRAY)
{
}

BufferObject::BufferObject(GLenum type)
    : mType(type),
      mPattern(GL_STATIC_DRAW),
      mBufferObjectId(0)
{
}

BufferObject::~BufferObject()
{
    destroy();
}

void BufferObject::allocate(const void *data, int count)
{
    if (!isCreated())
        return;

    glBufferData(mType, count, data, mPattern);
}

void BufferObject::create()
{
    glGenBuffers(1, &mBufferObjectId);
}

void BufferObject::bind()
{
    if (mBufferObjectId != 0)
        glBindBuffer(mType, mBufferObjectId);
}

void BufferObject::release()
{
    glBindBuffer(mType, 0);
}

void BufferObject::destroy()
{
    if (mBufferObjectId != 0) {
        glDeleteBuffers(1, &mBufferObjectId);
        mBufferObjectId = 0;
    }
}

