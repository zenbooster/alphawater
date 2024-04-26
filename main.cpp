#include <windows.h>
//#include <scrnsave.h>
#include <iostream>
#include <string>
#include <vector>
#include <io.h>
#include <fcntl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <csv.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

#include <GL/gl.h>

#include "common.h"
#include "shaderprogram.h"
#include "shaders.h"
#include "shaderinput.h"
#include "vertexarrayobject.h"
#include "bufferobject.h"
#include "framebuffer.h"

#define DATA_FOLDER "../data"
#define PACK_NAME "curling-smoke"
#define TEST_BUF_A

using namespace std;

#ifdef TEST_BUF_A
const char *fragmentShaderSource = 
R"(
// Curling Smoke by Leon Denise 2023-01-19

// finally learnt how to curl noise

// from Pete Werner article:
// http://petewerner.blogspot.com/2015/02/intro-to-curl-noise.html


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // coordinates
    vec2 uv = fragCoord/iResolution.xy;
   
    // frame
    vec3 color = texture(iChannel0, uv).rgb;

    fragColor = vec4(color,1.0);
}
)";

const char *fragmentShaderSource_buffer_a = 
R"(

// Curling Smoke

// finally learnt how to curl noise

// from Pete Werner article:
// http://petewerner.blogspot.com/2015/02/intro-to-curl-noise.html

#define R iResolution.xy
float gyroid (vec3 p) { return dot(sin(p),cos(p.yzx)); }
float noise (vec3 p)
{
    float result = 0., a = .5;
    float count = R.y < 500. ? 6. : 8.;
    for (float i = 0.; i < count; ++i, a/=2.)
    {
        p.z += iTime*.1;//+result*.5;
        result += abs(gyroid(p/a))*a;
    }
    return result;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 color = vec3(0);
    
    // coordinates
    vec2 uv = fragCoord/R.xy;
    vec2 p = (2.*fragCoord-R.xy)/R.y;
    vec2 offset = vec2(0);
    
    // curl
    vec2 e = vec2(.01,0);
    vec3 pos = vec3(p, length(p)*.5);
    float x = (noise(pos+e.yxy)-noise(pos-e.yxy))/(2.*e.x);
    float y = (noise(pos+e.xyy)-noise(pos-e.xyy))/(2.*e.x);
    vec2 curl = vec2(x,-y);

    // force fields
    offset += curl;
    offset -= normalize(p) * sin(iTime*2.-length(p)*6.);

    // displace buffer sampler coordinates
    uv += offset*.002*vec2(R.y/R.x, 1);
    vec3 frame = texture(iChannel0, uv).rgb;
    
    // spawn from edge
    bool spawn = fragCoord.x < 1. || fragCoord.x > R.x - 1.
        || fragCoord.y < 1. || fragCoord.y > R.y - 1.;
    
    // spawn at first frame
    spawn = spawn || iFrame < 1;
    
    // color palette
    // https://iquilezles.org/articles/palettes
    if (spawn) color = .5+.5*cos(vec3(1,2,3)*5.5+iTime+(uv.x+uv.y)*6.);
    
    // buffer
    else color = max(color, frame);
    
    fragColor = vec4(color,1.0);
})";

#else
const char *fragmentShaderSource = 
R"(
float M(inout vec3 s, inout vec3 q, float t)
{
  vec4 v = vec4(0, 33, 55, 0);
  vec4 z = cos( v + t*.4);
  mat2 m0 = mat2(z.x, z.y, z.z, z.w);
  z = cos( v + t*.3);
  mat2 m1 = mat2(z.x, z.y, z.z, z.w);

  s.xz *= m0;
  s.xy *= m1;
  float a = sin(q+sin(q+sin( q ).y).z).x*.5 - 1.;
  q = s + s + t;
  return length(s+sin(t*.7)) * log(length(s)+1.) + a;
}

void mainImage(out vec4 o, vec2 u)
{
    float z = 0.0;
    o *= 0.;
    vec3 q, p, s;
    vec2 R = iResolution.xy;
    float t = mod(iTime, 251.328);
    float d = 2.5;
    float r;
    for(; z < 7.; z++ )
    {
        s = p = vec3((u - .5 * R.xy) / R.y * d, 5. - d);
        r = M(s, q, t);
        d += min(r, 1.);
        s = p + .1;
        o += max(.7-r*.28 ,0.);
        o *= vec4(.1, .3, .4,0) - vec4(10, 5, 6,0) * (M(s, q, t) - r) / 4.;
    }
    o.a = 1.0;
} 
)";
#endif

class TMyApp;

class TMyAppWnd
{
	friend class TMyApp;

private:
	TMyApp *p_app;
	GLFWwindow *wnd;
	ShaderInput input;

	ShaderProgram *p_prg;
#ifdef TEST_BUF_A
	ShaderProgram *p_prg_a;
#endif

	VertexArrayObject *p_vao;
	BufferObject *p_vbo_arr;
	BufferObject *p_vbo_idx;

#ifdef TEST_BUF_A
	FrameBuffer *p_fbo[2];
	int i_fbo_idx;
#endif

	bool is_should_close();
	void initilizeUniformValue(int width, int height);
	void load(string pack_name);
	void init_wnd(int width, int height);
	void on_size(int width, int height);
	void on_key(int key, int scancode, int action, int mods);
	void on_mouse_pos(double xpos, double ypos);
	void on_mouse_btn(int button, int action, int mods);
	
public:
	TMyAppWnd(TMyApp *p_app, int width, int height, string caption, GLFWmonitor *mon = nullptr);
	~TMyAppWnd();
	void draw(void);
};

class TMyApp
{
	friend class TMyAppWnd;

	private:
		static const glm::vec2 screen;
		static const char caption[];
		static GLfloat vertices[];
		static GLuint indices[];
		bool is_parent_console;
		bool is_fullscreen;
		bool is_screensaver;
		int i_mon_cnt;
		int i_wnd_cnt;
		GLFWmonitor** mon;
		int wnd_pos[2], wnd_size[2];

		TMyAppWnd** wnd;
		//float *pf_time;
		float lastTime;
		float delta;

		bool is_any_wnd_should_close();
		void set_mode(void);
		void draw(void);
		inline bool is_preview(void) const;
		void init(bool is_screensaver, bool is_fullscreen, bool is_visible);
		void show_usage(void);

	public:
		//TMyApp(bool is_visible = true);
		TMyApp(int argc, char *argv[]);
		~TMyApp();
		
		void run(void);
}; // class TMyApp

const glm::vec2 TMyApp::screen(1, 1);

const char TMyApp::caption[] = "alphawater";

GLfloat TMyApp::vertices[] = {
   -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  0.0f,  0.0f,  1.0f,
   -1.0f, -1.0f,  0.0f,  0.0f,  0.0f
};

GLuint TMyApp::indices[] = {
    0, 1, 2,
    0, 2, 3
};

bool TMyAppWnd::is_should_close()
{
	bool res = glfwWindowShouldClose(wnd);

	return res;
}

TMyAppWnd::TMyAppWnd(TMyApp *p_app, int width, int height, string caption, GLFWmonitor *mon):
	p_app(p_app)
{
	wnd = glfwCreateWindow(width, height, caption.c_str(), mon, nullptr);
	if (!wnd)
	{
		throw invalid_argument("failed to create wnd");
	}

	if(mon)
	{
		glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	
	init_wnd(width, height);
}

TMyAppWnd::~TMyAppWnd()
{
	glfwDestroyWindow(wnd);
}

void TMyAppWnd::initilizeUniformValue(int width, int height)
{
    input.iResolution  = glm::vec3(width, height, 1.0f);
    input.iTime        = 0.0f;
    input.iGlobalTime  = 0.0f;
    input.iMouse       = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    input.iDate        = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    input.iSampleRate  = 44100 * 1.0f;

    /*auto size = mTextures.size();

    if (size <= 4)
    {
        for (int i = 0; i < size; i++)
        {
            input.iChannelResolution[i] = Vector3(mTextures[i]->width(),
                                                   mTextures[i]->height(),
                                                   1.0f);
            input.iChannelTime[i] = 0.0f;
        }
    }*/

    input.iFrame       = 0;
    input.iTimeDelta   = 1.0f;
    input.iFrameRate   = 0.0f;
}

void TMyAppWnd::load(string pack_name)
{
	string s_pack_folder = tostringstream() << DATA_FOLDER << "/" << pack_name + "/";
	enum TEnumFile
	{
		//ef_config,
		ef_common,
		ef_image,
		ef_buf_a,
		ef_buf_b,
		ef_buf_c,
		ef_buf_d
	};
	static map<TEnumFile, string> m_file_names =
	{
		//{ef_config, "config.csv"},
		{ef_common, "common.f"},
		{ef_image, "image.f"},
		{ef_buf_a, "buffer-a.f"},
		{ef_buf_b, "buffer-b.f"},
		{ef_buf_c, "buffer-c.f"},
		{ef_buf_d, "buffer-d.f"}
	};
	map<TEnumFile, string> m_file_content;
	string fname, fspec;
	bool is_has_buffers = false;

	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("app"));
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Загружаем пакет из каталога: ") << s_pack_folder);

	for(int i = ef_common; i <= ef_buf_d; i++)
	{
		TEnumFile ef = static_cast<TEnumFile>(i);
		fname = m_file_names[ef];
		fspec = tostringstream() << s_pack_folder << fname;

		ifstream f(fspec);
		if(f.good())
		{
			m_file_content[ef] = tostringstream() << f.rdbuf();
			LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Загружен файл: ") << fname);
			
			if(!is_has_buffers && ef >= ef_buf_a)
			{
				is_has_buffers = true;
			}
		}
		else
		{
			if(ef == ef_image)
			{
				LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Не найден обязательный файл: ") << fname);
				throw exception();
			}
		}
	}
	
	if(is_has_buffers)
	{
		/*if(!m_file_content.count(ef_config))
		{
			LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Есть буферы но нет конфига: ") << s_pack_folder);
			throw exception();
		}*/
		fname = "config.csv";
		fspec = tostringstream() << s_pack_folder << fname;
		LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Загружаем конфиг: ") << fname);
		io::CSVReader<5> in(fspec.c_str());
		in.read_header(io::ignore_missing_column, "prg", "ch0", "ch1", "ch2", "ch3");
		string prg, ch0, ch1, ch2, ch3;
		while(in.read_row(prg, ch0, ch1, ch2, ch3))
		{
			LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("") << prg << ": " << ch0 << ", " << ch1 << ", " << ch2 << ", " << ch3);
		}
	}
	
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Пакет загружен успешно!"));
}

void TMyAppWnd::init_wnd(int width, int height)
{
	glfwSetWindowUserPointer(wnd, this);
	{
		auto cb = [](GLFWwindow* wnd, int width, int height)
		{
			TMyAppWnd *o = reinterpret_cast<TMyAppWnd *>(glfwGetWindowUserPointer(wnd));
			o->on_size(width, height);
		};
		glfwSetFramebufferSizeCallback(wnd, cb);
	}
	{
		auto cb = [](GLFWwindow* wnd, int key, int scancode, int action, int mods)
		{
			TMyAppWnd *o = reinterpret_cast<TMyAppWnd *>(glfwGetWindowUserPointer(wnd));
			o->on_key(key, scancode, action, mods);
		};	
		glfwSetKeyCallback(wnd, cb);
	}

	if(p_app->is_screensaver)
	{
		{
			auto cb = [](GLFWwindow* wnd, double xpos, double ypos)
			{
				TMyAppWnd *o = reinterpret_cast<TMyAppWnd *>(glfwGetWindowUserPointer(wnd));
				o->on_mouse_pos(xpos, ypos);
			};
			glfwSetCursorPosCallback(wnd, cb);
		}
		{
			auto cb = [](GLFWwindow* wnd, int button, int action, int mods)
			{
				TMyAppWnd *o = reinterpret_cast<TMyAppWnd *>(glfwGetWindowUserPointer(wnd));
				o->on_mouse_btn(button, action, mods);
			};
			glfwSetMouseButtonCallback(wnd, cb);
		}
	}

	glfwMakeContextCurrent(wnd);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cerr << "failed to initialize glad with processes" << endl;
		exit(-1);
	}

	p_vao = new VertexArrayObject();
	p_vao->create();
	p_vao->bind();
	
    p_vbo_arr = new BufferObject(GL_ARRAY_BUFFER);
    p_vbo_arr->create();
    p_vbo_arr->bind();
    p_vbo_arr->setUsagePattern(GL_STATIC_DRAW);
    p_vbo_arr->allocate(p_app->vertices, sizeof(p_app->vertices));
	
	initilizeUniformValue(width, height);
	
	//load(PACK_NAME);

	p_prg = new ShaderProgram();
	p_prg->addShaderFromSource(Shader::ShaderType::Vertex, vertexShader);
	{
		std::string fragment = std::string(fragmentShaderPassHeader);
		
	#ifdef TEST_BUF_A
		char buffer[0x20];
		sprintf(buffer, "uniform sampler2D iChannel%d;\n", 0);
		fragment.append(buffer);
	#endif	
		fragment.append(fragmentShaderSource);
		fragment.append(fragmentShaderPassFooter);
		p_prg->addShaderFromSource(Shader::ShaderType::Fragment, fragment.c_str());
	}
    p_prg->link();

	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("vid"));
    if (!p_prg->isLinked())
	{
        LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("Не удалось слинковать шейдеры: ") << p_prg->log());
		throw exception();
	}
	
	p_prg->bind();
	p_prg->setUniformValue("iResolution", input.iResolution);

    p_prg->enableAttributeArray(0);
    p_prg->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(GLfloat) * 5);
    p_prg->enableAttributeArray(1);
    p_prg->setAttributeBuffer(1, GL_FLOAT, 3, 2, 5 * sizeof(GLfloat));
	
	p_prg->release();

#ifdef TEST_BUF_A
	i_fbo_idx = 0;

	p_prg_a = new ShaderProgram();
	p_prg_a->addShaderFromSource(Shader::ShaderType::Vertex, vertexShader);
	{
		std::string fragment = std::string(fragmentShaderPassHeader);
		
		char buffer[0x20];
		sprintf(buffer, "uniform sampler2D iChannel%d;\n", 0);
		fragment.append(buffer);

		fragment.append(fragmentShaderSource_buffer_a);
		fragment.append(fragmentShaderPassFooter);
		p_prg_a->addShaderFromSource(Shader::ShaderType::Fragment, fragment.c_str());
	}
    p_prg_a->link();

    if (!p_prg_a->isLinked())
	{
        LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("Не удалось слинковать шейдеры (buf A): ") << p_prg_a->log());
		throw exception();
	}
	
	p_prg_a->bind();
	p_prg_a->setUniformValue("iResolution", input.iResolution);

    p_prg_a->enableAttributeArray(0);
    p_prg_a->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(GLfloat) * 5);
    p_prg_a->enableAttributeArray(1);
    p_prg_a->setAttributeBuffer(1, GL_FLOAT, 3, 2, 5 * sizeof(GLfloat));
	p_prg_a->release();
#endif

    p_vbo_idx = new BufferObject(GL_ELEMENT_ARRAY_BUFFER);
    p_vbo_idx->create();
    p_vbo_idx->setUsagePattern(GL_STATIC_DRAW);
    p_vbo_idx->bind();
    p_vbo_idx->allocate(p_app->indices, sizeof(p_app->indices));

	p_vao->release();
	//p_prg_a->release();

#ifdef TEST_BUF_A	
	p_fbo[0] = new FrameBuffer();
	p_fbo[0]->create(width, height, false);
	p_fbo[1] = new FrameBuffer();
	p_fbo[1]->create(width, height, false);
#endif

	//glUniform2fv(glGetUniformLocation(shaderProgram, "iResolution"), 1, &screen[0]);
}

void TMyApp::set_mode(void)
{
    if (is_fullscreen)
    {
        // backup wnd position and wnd size
        glfwGetWindowPos(wnd[0]->wnd, &wnd_pos[0], &wnd_pos[1] );
        glfwGetWindowSize(wnd[0]->wnd, &wnd_size[0], &wnd_size[1] );

        // get resolution of monitor
        const GLFWvidmode * mode = glfwGetVideoMode(mon[0]);
        // switch to full screen
        glfwSetWindowMonitor(wnd[0]->wnd, mon[0], 0, 0, mode->width, mode->height, 0);
		glfwSetInputMode(wnd[0]->wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		i_wnd_cnt = i_mon_cnt;
		for(int i = 1; i < i_wnd_cnt; i++)
		{
			int width;
			int height;

			mode = glfwGetVideoMode(mon[i]);
			width = mode->width;
			height = mode->height;

			wnd[i] = new TMyAppWnd(this, width, height, caption, mon[i]);
			/*if(i & 1)
			{
				wnd[i]->input.iTimeDelta = -wnd[i]->input.iTimeDelta;
			}*/
			wnd[i]->on_size(width, height);
			//init_wnd(wnd[i], width, height);
		}
    }
    else
    {
		for(int i = 1; i < i_wnd_cnt; i++)
		{
			delete wnd[i];
			wnd[i] = NULL;
		}

		i_wnd_cnt = 1;
        // restore last wnd size and position
        glfwSetWindowMonitor(wnd[0]->wnd, nullptr,  wnd_pos[0], wnd_pos[1], wnd_size[0], wnd_size[1], 0);

		glfwSetInputMode(wnd[0]->wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void TMyAppWnd::on_size(int width, int height)
{
	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("wnd=") << wnd << LOG4CPLUS_TEXT("; w=") << width << LOG4CPLUS_TEXT("; h=") << height);

	glfwMakeContextCurrent(wnd);
	int t = max(width, height);
	glViewport(-(t-width)*0.5, -(t-height)*0.5, t, t);
	//glViewport(0, 0, t, t);
    //glViewport(0, 0, width, height);

	input.iResolution = glm::vec3(width, height, 1.0f);
	p_prg->bind();
	p_prg->setUniformValue("iResolution", input.iResolution);
	p_prg->release();

#ifdef TEST_BUF_A
	p_prg_a->bind();
	p_prg_a->setUniformValue("iResolution", input.iResolution);
	p_prg_a->release();

	for (int i = 0; i < 2; i++)
	{
		p_fbo[i]->resize(width, height, Texture::TEnumResizeContent::ercFromCenter);
	}
#endif

	if(!p_app->is_fullscreen)
	{
		draw();
	}
}

void TMyAppWnd::on_key(int key, __attribute__((unused)) int scancode, int action, int mods)
{
	static bool is_mode_switch = false;

	if(p_app->is_screensaver)
	{
		if(!p_app->is_preview())
		{
			glfwSetWindowShouldClose(wnd, true);
		}
	}
	else
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(wnd, true);
		}
		else
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && (mods & GLFW_MOD_ALT))
		{
			if(!is_mode_switch)
			{
				is_mode_switch = true;
				p_app->is_fullscreen = !p_app->is_fullscreen;
				p_app->set_mode();
			}
		}
		else
		{
			is_mode_switch = false;
		}
	}
}

void TMyAppWnd::on_mouse_pos(__attribute__((unused)) double xpos, __attribute__((unused)) double ypos)
{
	static bool is_first_run = true;
	
	if(p_app->is_screensaver && !p_app->is_preview())
	{
		if(is_first_run)
		{
			is_first_run = false;
		}
		else
		{
			glfwSetWindowShouldClose(wnd, true);
		}
	}
}

void TMyAppWnd::on_mouse_btn(__attribute__((unused)) int button, __attribute__((unused)) int action, __attribute__((unused)) int mods)
{
	if(p_app->is_screensaver && !p_app->is_preview())
	{
		glfwSetWindowShouldClose(wnd, true);
	}
}

void TMyAppWnd::draw(void)
{
	glfwMakeContextCurrent(wnd);
	
#ifdef TEST_BUF_A
	int channel = 0;
	shared_ptr<Texture> texture = p_fbo[i_fbo_idx]->texture();
	i_fbo_idx = (i_fbo_idx + 1) & 1;

	p_fbo[i_fbo_idx]->bind();
	p_prg_a->bind();
	
	texture->bindToChannel(channel);

	p_prg_a->setUniformValue("iTime", input.iTime);
	p_prg_a->setUniformValue("iFrame", input.iFrame);
	p_vao->bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	p_vao->release();
	glBindTexture(GL_TEXTURE_2D, 0);
	p_prg_a->release();
	p_fbo[i_fbo_idx]->release();
#endif	

	p_prg->bind();

#ifdef TEST_BUF_A
	channel = 0;
	p_fbo[i_fbo_idx]->texture()->bindToChannel(channel);
#endif

	p_prg->setUniformValue("iTime", input.iTime);
	p_prg->setUniformValue("iFrame", input.iFrame);
	p_vao->bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	p_vao->release();
#ifdef TEST_BUF_A
	glBindTexture(GL_TEXTURE_2D, 0);
#endif
	p_prg->release();

	glfwMakeContextCurrent(NULL);
	glfwSwapBuffers(wnd);
	
	input.iTime += input.iTimeDelta * p_app->delta;
	input.iFrame++;
}

void TMyApp::draw(void)
{
	float now = glfwGetTime();
	delta = now - lastTime;

	lastTime = now;

	for(int i = 0; i < i_wnd_cnt; i++)
	{
		wnd[i]->draw();
	}
}

inline bool TMyApp::is_preview(void) const
{
	return is_screensaver && !is_fullscreen;
}

void TMyApp::init(bool is_screensaver, bool is_fullscreen, bool is_visible)
{
    int width;
    int height;
	
	this->is_screensaver = is_screensaver;
	this->is_fullscreen = is_fullscreen;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

	if(!is_visible)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}

	mon = glfwGetMonitors(&i_mon_cnt);
	cout << i_mon_cnt << " monitors found." << endl;
	//wnd = new GLFWwindow * [i_mon_cnt];
	wnd = new TMyAppWnd * [i_mon_cnt];
	//pf_time = new float[i_mon_cnt];
	//for(int i = 0; i < i_mon_cnt; i++)
	//{
		//pf_time[i] = 0; //rand() % 100;
	//}
	
	if (is_fullscreen)
	{
		i_wnd_cnt = i_mon_cnt;
		for(int i = i_wnd_cnt - 1; i >= 0; i--)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(mon[i]);
			width = mode->width;
			height = mode->height;
			cout << "mon[" << i << "] = " << mon[i] << endl;
			wnd[i] = new TMyAppWnd(this, width, height, caption, mon[i]);
			/*if(i & 1)
			{
				wnd[i]->input.iTimeDelta = -wnd[i]->input.iTimeDelta;
			}*/
		}

		if(i_wnd_cnt > 0)
		{
			int dt = 100;
			wnd_pos[0] = dt;
			wnd_pos[1] = dt;
			dt <<= 1;
			wnd_size[0] = width - dt;
			wnd_size[1] = height - dt;
		}
	}
	else
	{
		i_wnd_cnt = 1;
		width = 200;
		height = 200;
		
		for(int i = i_wnd_cnt - 1; i >= 0; i--)
		{
			string cap = caption;
			cap += " ";
			cap += to_string(i);
			wnd[i] = new TMyAppWnd(this, width, height, caption);
		}

        glfwGetWindowSize(wnd[0]->wnd, &wnd_size[0], &wnd_size[1]);
        glfwGetWindowPos(wnd[0]->wnd, &wnd_pos[0], &wnd_pos[1]);
	}

	//for(int i = i_wnd_cnt - 1; i >= 0; i--)
	//{
		//MessageBoxA(NULL, "debug", "HIT.1", MB_OK);
		//init_wnd(wnd[i], width, height);

		lastTime = glfwGetTime();
		//delta = 1.0;
	//}
}

void TMyApp::show_usage(void)
{
	string s = "Неверные параметры!\n"
			"Использование:\n"
			"alphawater.exe [/c] | [/p <HWND>] | [/s]\n"
			"\t/c             - показать диалог настроек\n"
			"\t/p <HWND>      - предпросмотр в окне, дочернем по отношению к <HWND>\n"
			"\t/s             - запуск в полноэкранном режиме\n\n"
			"\tБез параметров - запуск в оконном режиме.";
			
	cout << s << endl;
}

TMyApp::TMyApp(int argc, char *argv[])
{
	switch(argc)
	{
		case 1 + 2:
		{
			if(!strcmp(argv[1], "/p"))
			{
				init(true, false, false);

				char *s_ptr = argv[2];
				HWND h_wnd_parent = (HWND)stoull(s_ptr, nullptr, 10);
				HWND h_wnd = glfwGetWin32Window(wnd[0]->wnd);
				SetParent(h_wnd, h_wnd_parent);
				SetWindowLong(h_wnd, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN);
				RECT rc;
				GetClientRect(h_wnd_parent, &rc);
				MoveWindow(h_wnd, rc.left, rc.top, rc.right, rc.bottom, TRUE);
			}
			else
			{
				show_usage();
				throw exception();
			}
			break;
		}
		
		case 1 + 1:
		{
			if(!strcmp(argv[1], "/s"))
			{
				init(true, true, true);
			}
			else
			if(!strcmp(argv[1], "/c"))
			{
			}
			else
			{
				show_usage();
				throw exception();
			}
			break;
		}
		
		default:
		{
			init(false, false, true);
		}
	}
}

TMyApp::~TMyApp()
{
	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("exit"));
	
	if(is_parent_console)
	{
		//MessageBoxA(NULL, to_string(FreeConsole()).c_str(), "debug", MB_OK);
		FreeConsole();
	}

    glfwTerminate();
    // cleanup
}

bool TMyApp::is_any_wnd_should_close()
{
	bool res = false;

	for(int i = i_wnd_cnt - 1; i >= 0; i--)
	{
		if(wnd[i]->is_should_close())
		{
			res = true;
			break;
		}
	}
	return res;
}

void TMyApp::run(void)
{
    while (!is_any_wnd_should_close())
    {
		draw();
        glfwPollEvents();
    }
}

int main(int argc, char *argv[])
{
	int res;

    log4cplus::Initializer initializer;

    //log4cplus::BasicConfigurator config;
    //config.configure();
	log4cplus::SharedAppenderPtr fileAppender(new log4cplus::FileAppender(LOG4CPLUS_TEXT("./main.log"),std::ios_base::app,true,true));
	fileAppender->setName(LOG4CPLUS_TEXT("file"));
	log4cplus::tstring pattern = LOG4CPLUS_TEXT("%D{%Y/%m/%d %H:%M:%S,%Q} [%t] %-5p %c - %m [%l]%n");
    fileAppender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));
	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
	logger.addAppender(fileAppender);
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("vid"));
	logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
	logger.addAppender(fileAppender);
	
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("start"));

	try
	{
		TMyApp app(argc, argv);
		app.run();
		res = 0;
	}
	catch(exception& )
	{
		res = 1;
	}
	
	return res;
}