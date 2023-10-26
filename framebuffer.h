#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "common.h"
#include "texture.h"

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    void resize(GLint w, GLint h, Texture::TEnumResizeContent erc = Texture::TEnumResizeContent::ercNone);
    void create(GLint w, GLint h, bool depth = true);
    void bind();
    void release();

    GLuint frameBufferId() const { return mFrameBufferId; }
    GLuint textureId() const { return mRenderTextureId; }
	Texture &texture() { return tex; }

    GLint width() const { return mRenderBufferWidth; }
    GLint height() const { return mRenderBufferHeight; }

private:
    GLuint mFrameBufferId, mOldFrameBufferId;
    GLuint mRenderTextureId;
    GLuint mDepthBuffer;

    bool mAllocated, mBinded;
    GLint mRenderBufferWidth, mRenderBufferHeight;
	
	Texture tex;
};

#endif // GLFRAMEBUFFER_H
