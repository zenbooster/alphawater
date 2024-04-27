#include "TMPShader.h"

FrameBufferPair::FrameBufferPair(int width, int height):
	i_fbo_idx(0)
{
	for(int i = 0; i < 2; i++)
	{
		fbo[i].create(width, height, false);
	}
};

void TMPShader::log_unassigned(uint32_t channel)
{
	LOG4CPLUS_INFO(logger, (string)*m_channels[channel] << " отсоединён от канала " << channel << " объекта " << (string)*this);
}

TMPShader::TMPShader(TMyAppWnd *wnd, string name, string vsh, string fsh):
	wnd(wnd),
	p_fbp(nullptr),
	i_assign_cnt(0),
	name(name),
	vsh(vsh),
	fsh(fsh),
	p_prg(nullptr)
{
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("app"));
	LOG4CPLUS_INFO(logger, "Создан " << (string)*this);

}

TMPShader::~TMPShader()
{
	if(p_prg)
	{
		delete p_prg;
	}
	LOG4CPLUS_INFO(logger, "Удалён " << (string)*this);
}

TMPShader::operator string() const
{
	return tostringstream() << "TMPShader(, \"" << name << "\", , ); this=" << this;
}

void TMPShader::assign(uint32_t channel, TMPShader *other)
{
	if(other)
	{
		if(m_channels.count(channel))
		{
			log_unassigned(channel);
		}
		else
		{
			if(!i_assign_cnt)
			{
				p_fbp = new FrameBufferPair(wnd->width, wnd->height);
			}
			i_assign_cnt++;
		}

		m_channels[channel] = other;
		LOG4CPLUS_INFO(logger, (string)*other << " присоединён к каналу " << channel << " объекта " << (string)*this);
	}
	else
	{
		if(m_channels.count(channel))
		{
			log_unassigned(channel);
			m_channels.erase(channel);
			if(i_assign_cnt)
			{
				delete p_fbp;
				p_fbp = nullptr;
				i_assign_cnt--;
			}
		}
	}
}

void TMPShader::link()
{
	if(p_prg)
	{
		delete p_prg;
	}
	
	p_prg = new ShaderProgram();
	p_prg->addShaderFromSource(Shader::ShaderType::Vertex, vsh);
	{
		stringstream fragment;

		fragment << fragmentShaderPassHeader;
		for(auto v : m_channels)
		{
			fragment << "uniform sampler2D iChannel" << v.first;
	
			TMPShader *other = v.second;
			if(other != this)
			{
				other->link();
			}
		}
		fragment << fsh;
		fragment << fragmentShaderPassFooter;
		p_prg->addShaderFromSource(Shader::ShaderType::Fragment, fragment.str().c_str());
	}
    p_prg->link();
}