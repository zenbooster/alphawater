#include "TMPShader.h"
#include "TMyAppWnd.h"
#include <map>
#include <stack>

void TMPShader::log_unassigned(uint32_t channel)
{
	LOG4CPLUS_INFO(logger, (string)*m_channels[channel] << " отсоединён от канала " << channel << " объекта " << (string)*this);
}

void TMPShader::traverse(bool visited_value, function<void (TMPShader *)> cb_next, function<void (pair<uint32_t, TMPShader *>)> cb)
{
	/*for(auto v : m_channels)
	{
		cb(v);

		TMPShader *other = v.second;
		if(other != this)
		{
			cb_next(other);
		}
	}*/
	
	//map<TMPShader *, bool > visited;
	//stack<TMPShader *> st;
	
	visited = visited_value;

	for(auto v : m_channels)
	{
		cb(v);

		TMPShader *other = v.second;
		if(other->visited != visited_value)
		{
			//other->traverse(visited_value, cb_next, cb);
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
	p_prg(nullptr),
	visited(false)
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
	//return tostringstream() << "TMPShader(, \"" << name << "\", , ); this=" << this;
	return tostringstream() << "TMPShader(, \"" << name << "\", , )";
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
			if(!other->i_assign_cnt)
			{
				other->p_fbp = new TFrameBufferPair(wnd->width, wnd->height);
			}
			other->i_assign_cnt++;
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
			if(other->i_assign_cnt)
			{
				delete other->p_fbp;
				other->p_fbp = nullptr;
				other->i_assign_cnt--;
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
		traverse(!visited, [](TMPShader *o){o->link();}, [&fragment](pair<uint32_t, TMPShader *> v){fragment << "uniform sampler2D iChannel" << v.first << ";" << endl;});
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
	traverse(!visited, [width, height](TMPShader *o){o->resize(width, height);});

	p_prg->bind();
	p_prg->setUniformValue("iResolution", wnd->input.iResolution);
	p_prg->release();
	
	if(p_fbp)
	{
		p_fbp->resize(width, height);
	}
}

void TMPShader::draw()
{
	LOG4CPLUS_INFO(logger, "(" << (string)*this << ")->draw()");
	traverse(!visited, [](TMPShader *o){o->draw();});
	
	LOG4CPLUS_INFO(logger, (string)*this << " начало отрисовки");

	p_prg->bind();

	if(p_fbp)
	{
		p_fbp->bind();
		LOG4CPLUS_INFO(logger, (string)*this << " fb[" << p_fbp->i << "] binded");
	}

	for(auto v : m_channels)
	{
		uint32_t ch = v.first;
		TMPShader *other = v.second;
		string name = tostringstream() << "iChannel" << ch;
		GLint location = glGetUniformLocation(p_prg->programId(), name.c_str());
		other->p_fbp->texture()->bindToChannel(ch);
		//p_prg->setUniformValue(name.c_str(), ch);
		glUniform1i(location, ch);
		LOG4CPLUS_INFO(logger, (string)*this << " " << other->name << "->fb[" << other->p_fbp->i << "] texture binded to channel " << ch);
	}

	p_prg->setUniformValue("iTime", wnd->input.iTime);
	p_prg->setUniformValue("iFrame", wnd->input.iFrame);
	wnd->p_vao->bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	wnd->p_vao->release();
	p_prg->release();

	if(p_fbp)
	{
		p_fbp->release();
		LOG4CPLUS_INFO(logger, (string)*this << " fb[" << p_fbp->i << "] released");
		p_fbp->swap();
	}

	LOG4CPLUS_INFO(logger, (string)*this << " конец отрисовки");
}