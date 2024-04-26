#pragma once
#include <cstdint>
#include <map>
#include "TMyAppWnd.h"
#include "ShaderProgram.h"
#include "framebuffer.h"

struct FrameBufferPair
{
	FrameBuffer fbo[2];
	int i_fbo_idx;
	
	FrameBufferPair(int width, int height);
};

class TMPShader
{
	TMyAppWnd *wnd;
	map<uint32_t, TMPShader *> m_channels;
	FrameBufferPair *p_fbp;
	int i_assign_cnt;

	ShaderProgram *p_prg;

public:
	TMPShader(TMyAppWnd *wnd);
	
	void assign(uint32_t channel, TMPShader *other);
};