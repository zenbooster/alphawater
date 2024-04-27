#include "TMPShader.h"

void TMPShader::log_unassigned(uint32_t channel)
{
	LOG4CPLUS_INFO(logger, (string)*m_channels[channel] << " отсоединён от канала " << channel << " объекта " << (string)*this);
}

void TMPShader::traverse(function<void>(TMPShader *) cb_next, function<void>(pair<uint32_t, TMPShader *>) cb)
{
	for(auto v : m_channels)
	{
		cb(v);

		TMPShader *other = v.second;
		if(other != this)
		{
			cb_next(other);
		}
	}
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
				p_fbp = new TFrameBufferPair(wnd->width, wnd->height);
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
	LOG4CPLUS_INFO(logger, "(" << (string)*this << ")->link()");

	if(p_prg)
	{
		delete p_prg;
	}
	
	p_prg = new ShaderProgram();
	p_prg->addShaderFromSource(Shader::ShaderType::Vertex, vsh);
	{
		stringstream fragment;

		fragment << fragmentShaderPassHeader;
		traverse([](o){o->link();}, [](v){fragment << "uniform sampler2D iChannel" << v.first;})
		/*for(auto v : m_channels)
		{
			fragment << "uniform sampler2D iChannel" << v.first;
	
			TMPShader *other = v.second;
			if(other != this)
			{
				other->link();
			}
		}*/
		fragment << fsh;
		fragment << fragmentShaderPassFooter;
		p_prg->addShaderFromSource(Shader::ShaderType::Fragment, fragment.str().c_str());
	}
    p_prg->link();
    if (!p_prg->isLinked())
	{
        LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("Не удалось слинковать шейдеры: ") << p_prg->log());
		throw exception();
	}
	p_prg->bind();
	p_prg->setUniformValue("iResolution", wnd->input.iResolution);

    p_prg->enableAttributeArray(0);
    p_prg->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(GLfloat) * 5);
    p_prg->enableAttributeArray(1);
    p_prg->setAttributeBuffer(1, GL_FLOAT, 3, 2, 5 * sizeof(GLfloat));
	
	p_prg->release();
}

void TMPShader::resize(int width, int height)
{
	LOG4CPLUS_INFO(logger, "(" << (string)*this << ")->on_size(..)");
	traverse([](o){o->resize(width, height);});

	p_prg->bind();
	p_prg->setUniformValue("iResolution", wnd->input.iResolution);
	p_prg->release();
	
	p_fbp->resize(width, height);
}

void TMPShader::draw()
{
	LOG4CPLUS_INFO(logger, "(" << (string)*this << ")->draw()");
	traverse([](o){o->draw();});
	/*for(auto v : m_channels)
	{
		TMPShader *other = v.second;
		if(other != this)
		{
			other->draw();
		}
	}*/
	
	LOG4CPLUS_INFO(logger, (string)*this << " начало отрисовки");
	shared_ptr<Texture> texture;

	if(p_fbp)
	{
		texture = p_fbp->texture();
		p_fbp->swap();
		p_fbp->bind();
	}

	p_prg->bind();
	
	if(p_fbp)
	{
		texture->bindToChannel(channel);
	}

	p_prg->setUniformValue("iTime", input.iTime);
	p_prg->setUniformValue("iFrame", input.iFrame);
	wnd->p_vao->bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	wnd->p_vao->release();
	glBindTexture(GL_TEXTURE_2D, 0);
	p_prg->release();

	if(p_fbp)
	{
		p_fbp->release();
	}

	LOG4CPLUS_INFO(logger, (string)*this << " конец отрисовки");
}