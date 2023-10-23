#ifndef VERTEX_ARRAY_OBJECT_H
#define VERTEX_ARRAY_OBJECT_H

#include "common.h"

class VertexArrayObject
{
public:
    VertexArrayObject();
    ~VertexArrayObject();

    void create();
    void bind();
    void release();
    void destroy();

    bool isCreated() const { return mVertexArrayObjectId != 0; }
    GLuint objectId() const { return mVertexArrayObjectId; }

private:
    GLuint mVertexArrayObjectId;
};

#endif // VERTEX_ARRAY_OBJECT_H
