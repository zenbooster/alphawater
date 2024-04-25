#include "framebuffer.h"
#include "common.h"

FrameBuffer::FrameBuffer()
    : mFrameBufferId(0),
      mOldFrameBufferId(0),
      mDepthBuffer(0),
      mAllocated(false),
      mBinded(false),
      mRenderBufferWidth(0),
      mRenderBufferHeight(0),
	  tex(make_shared<Texture>(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR, false))
{
}

FrameBuffer::~FrameBuffer()
{
    release();
    if (mAllocated)
    {
        glDeleteRenderbuffers(1, &mDepthBuffer);
        glDeleteFramebuffers(1, &mFrameBufferId);
        mAllocated = false;
    }
}

void FrameBuffer::resize(GLint w, GLint h, Texture::TEnumResizeContent erc)
{
    bind();
	tex->resize(w, h, erc);

    if (mDepthBuffer != 0)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    release();

    mRenderBufferWidth = w;
    mRenderBufferHeight = h;
}

void FrameBuffer::create(GLint w, GLint h, bool depth)
{
    if (!mAllocated)
    {
        glGenFramebuffers(1, &mFrameBufferId);

        if (depth)
            glGenRenderbuffers(1, &mDepthBuffer);

    }

    mRenderBufferWidth = w;
    mRenderBufferHeight = h;

    bind();

	tex->createEmpty(w, h);

    if (depth)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        mAllocated = true;

    release();

    glBindTexture(GL_TEXTURE_2D, 0);
    if (depth)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void FrameBuffer::bind()
{
    if (!mBinded)
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&mOldFrameBufferId);

        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
        glViewport(0, 0, mRenderBufferWidth, mRenderBufferHeight);
        /*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        if (mDepthBuffer != 0)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else
            glClear(GL_COLOR_BUFFER_BIT);
		*/

        mBinded = true;
    }
}

void FrameBuffer::release()
{
    if (mBinded)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBufferId);
        mBinded = false;
    }
}