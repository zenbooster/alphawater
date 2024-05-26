#pragma once
#include <cstdint>
#include <map>
#include <functional>
#include <memory>
#include "ShaderProgram.h"
#include "TFrameBufferPair.h"

class TMyAppWnd;
class TMPShader;

struct TChannel
{
	TMPShader *mpshader;
	TTexParam par;
	
	TChannel(TMPShader *mpsh, TTexParam& par):
		mpshader(mpsh),
		par(par)
	{}
};

class TMPShader
{
	log4cplus::Logger logger;
	TMyAppWnd *wnd;
	map<uint32_t, shared_ptr<TChannel>> m_channels;
	TFrameBufferPair *p_fbp;
	int i_assign_cnt;
	
	string name;
	string vsh;
	string fsh;
	ShaderProgram *p_prg;
	bool visited;

	void log_unassigned(uint32_t channel);

public:
	TMPShader(TMyAppWnd *wnd, string name, string vsh, string fsh);
	~TMPShader();
	
	operator string() const;
	
	void traverse(bool visited_value, function<void (TMPShader *)> cb_next, function<void (pair<uint32_t, shared_ptr<TChannel>>)> cb = [](pair<uint32_t, shared_ptr<TChannel>>){});
	
	void assign(uint32_t channel, TMPShader *other, TTexParam& par, TTexParams& pars);
	void link();
	void resize(int width, int height);
	void draw();
	//void swap();
};