#include "TFrameBufferPair.h"
#include <utility>

TFrameBufferPair::TFrameBufferPair(int width, int height, TTexParams& pars):
	i(0),
	j(1),
	fbo{FrameBuffer(pars), FrameBuffer(pars)},
	pars(pars)
{
	for(int i = 0; i < 2; i++)
	{
		fbo[i].create(width, height, false);
	}
};

void TFrameBufferPair::resize(int width, int height)
{
	for (int i = 0; i < 2; i++)
	{
		fbo[i].resize(width, height, Texture::TEnumResizeContent::ercFromCenter);
	}
}

shared_ptr<Texture> TFrameBufferPair::texture(TTexParam par, bool is_swap)
{
	return fbo[is_swap ? i : j].texture(par);
}

void TFrameBufferPair::swap()
{
	std::swap(i, j);

	for(auto v : pars)
	{
		shared_ptr<Texture> tx = texture(v);
		if (tx->mFilterMode == GL_MIPMAP)
		{
			glGenerateTextureMipmap(tx->textureId());
		}
	}
}

void TFrameBufferPair::bind()
{
	fbo[i].bind();
}

void TFrameBufferPair::release()
{
	fbo[i].release();
}