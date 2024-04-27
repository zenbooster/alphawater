#include "TFrameBufferPair.h"

TFrameBufferPair::TFrameBufferPair(int width, int height):
	i(0)
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

shared_ptr<Texture> TFrameBufferPair::texture()
{
	return fbo[i].texture();
}

void TFrameBufferPair::swap()
{
	i = (i + 1) & 1;
}

void TFrameBufferPair::bind()
{
	fbo[i].bind();
}

void TFrameBufferPair::release()
{
	fbo[i].release();
}