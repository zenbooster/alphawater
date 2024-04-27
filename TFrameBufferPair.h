#pragma once
#include "framebuffer.h"
//#include "texture.h"

struct TFrameBufferPair
{
	FrameBuffer fbo[2];
	int i, j;
	
	TFrameBufferPair(int width, int height);
	
	void resize(int width, int height);
	shared_ptr<Texture> texture();
	void swap();
	void bind();
	void release();
};