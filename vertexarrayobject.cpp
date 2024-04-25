#include "vertexarrayobject.h"
#include "common.h"

VertexArrayObject::VertexArrayObject()
    : mVertexArrayObjectId(0)
{
}

VertexArrayObject::~VertexArrayObject()
{
    destroy();
}

void VertexArrayObject::create()
{
    glGenVertexArrays(1, &mVertexArrayObjectId);
}

void VertexArrayObject::bind()
{
    glBindVertexArray(mVertexArrayObjectId);
}

void VertexArrayObject::release()
{
    glBindVertexArray(0);
}

void VertexArrayObject::destroy()
{
    if (mVertexArrayObjectId != 0) {
        glDeleteVertexArrays(1, &mVertexArrayObjectId);
        mVertexArrayObjectId = 0;
    }
}



