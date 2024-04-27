#pragma once
#include <cstdint>
#include <map>
#include <functional>
#include "ShaderProgram.h"
#include "TFrameBufferPair.h"

class TMyAppWnd;

class TMPShader
{
	log4cplus::Logger logger;
	TMyAppWnd *wnd;
	map<uint32_t, TMPShader *> m_channels;
	TFrameBufferPair *p_fbp;
	int i_assign_cnt;

	string name;
	string vsh;
	string fsh;
	ShaderProgram *p_prg;
	
	void log_unassigned(uint32_t channel);
	void traverse(function<void (TMPShader *)> cb_next, function<void (pair<uint32_t, TMPShader *>)> cb = [](pair<uint32_t, TMPShader *>){});

public:
	TMPShader(TMyAppWnd *wnd, string name, string vsh, string fsh);
	~TMPShader();
	
	operator string() const;
	
	void assign(uint32_t channel, TMPShader *other);
	void link();
	void resize(int width, int height);
	void draw();
};