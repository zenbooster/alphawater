#include "vertexarrayobject.h"
#include "TGles2Fns.h"

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
    TGles2Fns::glGenVertexArrays(1, &mVertexArrayObjectId);
}

void VertexArrayObject::bind()
{
    TGles2Fns::glBindVertexArray(mVertexArrayObjectId);
}

void VertexArrayObject::release()
{
    TGles2Fns::glBindVertexArray(0);
}

void VertexArrayObject::destroy()
{
    if (mVertexArrayObjectId != 0) {
        TGles2Fns::glDeleteVertexArrays(1, &mVertexArrayObjectId);
        mVertexArrayObjectId = 0;
    }
}



