#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "common.h"

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    void create(GLint w, GLint h, bool depth = true);
    void bind();
    void release();

    GLuint frameBufferId() const { return mFrameBufferId; }
    GLuint textureId() const { return mRenderTextureId; }

    GLint width() const { return mRenderBufferWidth; }
    GLint height() const { return mRenderBufferHeight; }

private:
    GLuint mFrameBufferId, mOldFrameBufferId;
    GLuint mRenderTextureId;
    GLuint mDepthBuffer;

    bool mAllocated, mBinded;
    GLint mRenderBufferWidth, mRenderBufferHeight;
};

#endif // GLFRAMEBUFFER_H
