#include "bufferobject.h"
#include "TGles2Fns.h"

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

    TGles2Fns::glBufferData(mType, count, data, mPattern);
}

void BufferObject::create()
{
    TGles2Fns::glGenBuffers(1, &mBufferObjectId);
}

void BufferObject::bind()
{
    if (mBufferObjectId != 0)
        TGles2Fns::glBindBuffer(mType, mBufferObjectId);
}

void BufferObject::release()
{
    TGles2Fns::glBindBuffer(mType, 0);
}

void BufferObject::destroy()
{
    if (mBufferObjectId != 0) {
        TGles2Fns::glDeleteBuffers(1, &mBufferObjectId);
        mBufferObjectId = 0;
    }
}

