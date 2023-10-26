#include "framebuffer.h"
#include "TGles2Fns.h"

FrameBuffer::FrameBuffer()
    : mFrameBufferId(0),
      mOldFrameBufferId(0),
      mRenderTextureId(0),
      mDepthBuffer(0),
      mAllocated(false),
      mBinded(false),
      mRenderBufferWidth(0),
      mRenderBufferHeight(0),
	  tex(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR, false)
{
	mRenderTextureId = tex.textureId();
}

FrameBuffer::~FrameBuffer()
{
    release();
    if (mAllocated)
    {
        TGles2Fns::glDeleteTextures(1, &mRenderTextureId);
        TGles2Fns::glDeleteRenderbuffers(1, &mDepthBuffer);
        TGles2Fns::glDeleteFramebuffers(1, &mFrameBufferId);
        mAllocated = false;
    }
}

void FrameBuffer::resize(GLint w, GLint h, Texture::TEnumResizeContent erc)
{
    bind();
	tex.resize(w, h, erc);
	mRenderTextureId = tex.textureId();

    if (mDepthBuffer != 0)
    {
        TGles2Fns::glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
        TGles2Fns::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        TGles2Fns::glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    release();

    mRenderBufferWidth = w;
    mRenderBufferHeight = h;
}

void FrameBuffer::create(GLint w, GLint h, bool depth)
{
    if (!mAllocated)
    {
        TGles2Fns::glGenFramebuffers(1, &mFrameBufferId);

        if (depth)
            TGles2Fns::glGenRenderbuffers(1, &mDepthBuffer);

    }

    mRenderBufferWidth = w;
    mRenderBufferHeight = h;

    bind();

	tex.createEmpty(w, h);

    if (depth)
    {
        TGles2Fns::glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
        TGles2Fns::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        TGles2Fns::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
    }

    if (TGles2Fns::glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        mAllocated = true;

    release();

    TGles2Fns::glBindTexture(GL_TEXTURE_2D, 0);
    if (depth)
    {
        TGles2Fns::glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void FrameBuffer::bind()
{
    if (!mBinded)
    {
        TGles2Fns::glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&mOldFrameBufferId);

        TGles2Fns::glBindTexture(GL_TEXTURE_2D, 0);
        TGles2Fns::glEnable(GL_TEXTURE_2D);
        TGles2Fns::glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
        TGles2Fns::glViewport(0, 0, mRenderBufferWidth, mRenderBufferHeight);
        /*TGles2Fns::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        if (mDepthBuffer != 0)
            TGles2Fns::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else
            TGles2Fns::glClear(GL_COLOR_BUFFER_BIT);
		*/

        mBinded = true;
    }
}

void FrameBuffer::release()
{
    if (mBinded)
    {
        TGles2Fns::glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBufferId);
        mBinded = false;
    }
}
