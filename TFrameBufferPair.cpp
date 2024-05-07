#include "TFrameBufferPair.h"
#include <utility>

TFrameBufferPair::TFrameBufferPair(int width, int height):
	i(0), j(1)
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

shared_ptr<Texture> TFrameBufferPair::texture(bool is_swap)
{
	return fbo[is_swap ? i : j].texture();
}

void TFrameBufferPair::swap()
{
	std::swap(i, j);
}

void TFrameBufferPair::bind()
{
	fbo[i].bind();
}

void TFrameBufferPair::release()
{
	fbo[i].release();
}