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
      mRenderBufferHeight(0)
{
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

void FrameBuffer::create(GLint w, GLint h, bool depth)
{
    if (!mAllocated)
    {
        TGles2Fns::glGenFramebuffers(1, &mFrameBufferId);
        TGles2Fns::glGenTextures(1, &mRenderTextureId);

        if (depth)
            TGles2Fns::glGenRenderbuffers(1, &mDepthBuffer);

    }

    mRenderBufferWidth = w;
    mRenderBufferHeight = h;

    bind();

    TGles2Fns::glBindTexture(GL_TEXTURE_2D, mRenderTextureId);
    TGles2Fns::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    TGles2Fns::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    TGles2Fns::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    TGles2Fns::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TGles2Fns::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    TGles2Fns::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderTextureId, 0);

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
        TGles2Fns::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        if (mDepthBuffer != 0)
            TGles2Fns::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        else
            TGles2Fns::glClear(GL_COLOR_BUFFER_BIT);

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
