#include "TMPShader.h"

FrameBufferPair::FrameBufferPair(int width, int height):
	i_fbo_idx(0)
{
	for(int i = 0; i < 2; i++)
	{
		fbo[i].create(width, height, false);
	}
};

TMPShader::TMPShader(TMyAppWnd *wnd):
	wnd(wnd),
	p_fbp(nullptr),
	i_assign_cnt(0)
{
}

void TMPShader::assign(uint32_t channel, TMPShader *other)
{
	//if(!i_assign_cnt)
	if(m_channels.count(channel))
	{
	}
	else
	{
		m_channels[channel] = other;
		if(!i_assign_cnt)
		{
			p_fbp = new FrameBufferPair(wnd->width, wnd->height);
		}
		i_assign_cnt++;
	}
}