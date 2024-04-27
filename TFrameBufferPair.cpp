#include "TFrameBufferPair.h"

TFrameBufferPair::TFrameBufferPair(int width, int height):
	i_fbo_idx(0)
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
		p_fbo[i]->resize(width, height, Texture::TEnumResizeContent::ercFromCenter);
	}
}

shared_ptr<Texture> TFrameBufferPair::texture()
{
	return p_fbo[i_fbo_idx]->texture();
}

void TFrameBufferPair::swap()
{
	i_fbo_idx = (i_fbo_idx + 1) & 1;
}

void TFrameBufferPair::bind()
{
	p_fbo[i_fbo_idx]->bind();
}

void TFrameBufferPair::release()
{
	p_fbo[i_fbo_idx]->release();
}