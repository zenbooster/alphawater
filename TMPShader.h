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
	log4cplus::Logger logger;
	TMyAppWnd *wnd;
	map<uint32_t, TMPShader *> m_channels;
	FrameBufferPair *p_fbp;
	int i_assign_cnt;

	string name;
	string vsh;
	string fsh;
	ShaderProgram *p_prg;
	
	void log_unassigned(uint32_t channel);

public:
	TMPShader(TMyAppWnd *wnd, string name, string vsh, string fsh);
	~TMPShader();
	
	operator string() const;
	
	void assign(uint32_t channel, TMPShader *other);
	void link();
};