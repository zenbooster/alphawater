#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <memory>
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
    GLuint textureId() const { return tex->textureId(); }
	shared_ptr<TextureIdHolder> textureIdHolder() { return tex->textureIdHolder(); }
	shared_ptr<Texture> texture() { return tex; }

    GLint width() const { return mRenderBufferWidth; }
    GLint height() const { return mRenderBufferHeight; }

private:
    GLuint mFrameBufferId, mOldFrameBufferId;
    GLuint mDepthBuffer;

    bool mAllocated, mBinded;
    GLint mRenderBufferWidth, mRenderBufferHeight;
	
	shared_ptr<Texture> tex;
};

#endif // GLFRAMEBUFFER_H
