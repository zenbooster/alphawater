#pragma once
#include "framebuffer.h"
//#include "texture.h"

struct TFrameBufferPair
{
	int i, j;
	FrameBuffer fbo[2];
	
	TFrameBufferPair(int width, int height, TTexParams& pars);
	
	void resize(int width, int height);
	shared_ptr<Texture> texture(TTexParam par, bool is_swap = false);
	void swap();
	void bind();
	void release();
};