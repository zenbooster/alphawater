#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "common.h"
#include "texture.h"
#include "TTexParam.h"

class FrameBuffer
{
public:
	static const TTexParam def_par;
	FrameBuffer(TTexParams pars = {def_par});
    ~FrameBuffer();

    void resize(GLint w, GLint h, Texture::TEnumResizeContent erc = Texture::TEnumResizeContent::ercNone);
    void create(GLint w, GLint h, bool depth = true);
    void bind();
    void release();

    GLuint frameBufferId() const { return mFrameBufferId; }
    GLuint textureId(TTexParam par = def_par) { return tex[par]->textureId(); }
	shared_ptr<TextureIdHolder> textureIdHolder(TTexParam par = def_par) { return tex[par]->textureIdHolder(); }
	shared_ptr<Texture> texture(TTexParam par = def_par) { return tex[par]; }

    GLint width() const { return mRenderBufferWidth; }
    GLint height() const { return mRenderBufferHeight; }

private:
    GLuint mFrameBufferId, mOldFrameBufferId;
    GLuint mDepthBuffer;

    bool mAllocated, mBinded;
    GLint mRenderBufferWidth, mRenderBufferHeight;
	
	unordered_map<TTexParam, shared_ptr<Texture>, TTexParamHashFunction> tex;
};

#endif // GLFRAMEBUFFER_H
