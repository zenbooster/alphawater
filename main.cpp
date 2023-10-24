#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "TGles2Fns.h"
#include "shaderprogram.h"
#include "shaders.h"
#include "shaderinput.h"
#include "vertexarrayobject.h"
#include "bufferobject.h"
#include "framebuffer.h"

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
    
    // normal
    /*vec2 e = vec2(0, 0);
    #define T(u) texture(iChannel0, uv+u).r
    vec3 normal = vec3(
        T(e.xy)-T(-e.xy), 
        T(-e.yx)-T(e.yx),
        color.r*.1);
    if (abs(normal.x) + abs(normal.y) + abs(normal.z) > .001)
        normal = normalize(normal);
             
    // shade
    color *= dot(normal, normalize(vec3(0,1,1)))*.5+.5;
	*/
    
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

#define GL_CHECK(x)                                                                         \
    x;                                                                                      \
    {                                                                                       \
        GLenum glError = TGles2Fns::glGetError();                                                  \
        if (glError != GL_NO_ERROR) {                                                       \
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "glGetError() = %i (0x%.8x) at line %i\n", glError, glError, __LINE__); \
            exit(-1);                                                                        \
        }                                                                                   \
    }

class TMyApp
{
	private:
	    int width;
		int height;
		static float quadVerts[];
		static GLfloat vertices[];
		static GLuint indices[];
		bool is_running;
	    bool is_fullscreen;
		bool is_screensaver;
		//GLFWmonitor* mon;
		int wnd_pos[2], wnd_size[2];
		
		SDL_Window* wnd;
		SDL_GLContext ctx;
		int render_flags;
		ShaderProgram *p_prg;
	#ifdef TEST_BUF_A
		ShaderProgram *p_prg_a;
	#endif
		ShaderInput input;
		VertexArrayObject *p_vao;
		BufferObject *p_vbo_arr;
		BufferObject *p_vbo_idx;
	#ifdef TEST_BUF_A
		FrameBuffer *p_fbo[2];
		int i_fbo_idx;
	#endif
		float lastTime;

		void initilizeUniformValue();
		//void set_mode(void);
		void on_size(void);
		//void on_key(GLFWwindow* wnd, int key, int scancode, int action, int mods);
		//void on_mouse_pos(GLFWwindow* wnd, double xpos, double ypos);
		//void on_mouse_btn(GLFWwindow* wnd, int button, int action, int mods);
		void draw(void);
		inline bool is_preview(void) const;
		void init(bool is_screensaver, bool is_fullscreen, bool is_visible);
		void show_usage(void);

	public:
		TMyApp(int argc, char *argv[]);
		~TMyApp();
		
		void run(void);
};

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

void TMyApp::initilizeUniformValue()
{
    //input.iResolution  = Vector3(width() * 1.0f, height() * 1.0f, 1.0f);
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
    input.iTimeDelta   = 0.0f;
    input.iFrameRate   = 0.0f;
}

/*void TMyApp::set_mode(void)
{
    if (is_fullscreen)
    {
        // backup wnd position and wnd size
        glfwGetWindowPos(wnd, &wnd_pos[0], &wnd_pos[1] );
        glfwGetWindowSize(wnd, &wnd_size[0], &wnd_size[1] );
        
        // get resolution of monitor
        const GLFWvidmode * mode = glfwGetVideoMode(mon);

        // switch to full screen
        glfwSetWindowMonitor(wnd, mon, 0, 0, mode->width, mode->height, 0);

		glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        // restore last wnd size and position
        glfwSetWindowMonitor(wnd, nullptr,  wnd_pos[0], wnd_pos[1], wnd_size[0], wnd_size[1], 0 );

		glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}*/

void TMyApp::on_size(void)
{
	SDL_GetWindowSize(wnd, &width, &height);
	
	int status = SDL_GL_MakeCurrent(wnd, ctx);
	if (status)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
		exit(-1);
	}

    TGles2Fns::glViewport(0, 0, width, height);
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
		p_fbo[i]->resize(width, height);
	}
#endif
	SDL_GL_MakeCurrent(wnd, NULL);
}
/*
void TMyApp::on_key(GLFWwindow* wnd, int key, __attribute__((unused)) int scancode, int action, int mods)
{
	static bool is_mode_switch = false;

	if(is_screensaver)
	{
		if(!is_preview())
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
				is_fullscreen = !is_fullscreen;
				set_mode();
			}
		}
		else
		{
			is_mode_switch = false;
		}
	}
}

void TMyApp::on_mouse_pos(GLFWwindow* wnd, __attribute__((unused)) double xpos, __attribute__((unused)) double ypos)
{
	static bool is_first_run = true;
	
	if(is_screensaver && !is_preview())
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

void TMyApp::on_mouse_btn(GLFWwindow* wnd, __attribute__((unused)) int button, __attribute__((unused)) int action, __attribute__((unused)) int mods)
{
	if(is_screensaver && !is_preview())
	{
		glfwSetWindowShouldClose(wnd, true);
	}
}*/

void TMyApp::draw(void)
{
	float now = SDL_GetTicks() * 0.001;
	float delta = now - lastTime;

	lastTime = now;

	int status = SDL_GL_MakeCurrent(wnd, ctx);
	if (status)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
		exit(-1);
	}

#ifdef TEST_BUF_A
	int channel = 0;
	int i_tex = p_fbo[i_fbo_idx]->textureId();
	i_fbo_idx = (i_fbo_idx + 1) & 1;

	p_fbo[i_fbo_idx]->bind();
	p_prg_a->bind();
	
    TGles2Fns::glActiveTexture(GL_TEXTURE0 + channel);
	TGles2Fns::glBindTexture(GL_TEXTURE_2D, i_tex);

	p_prg_a->setUniformValue("iTime", input.iTime);
	p_prg_a->setUniformValue("iFrame", input.iFrame);
	p_vao->bind();
	TGles2Fns::glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	p_vao->release();
	TGles2Fns::glBindTexture(GL_TEXTURE_2D, 0);
	p_prg_a->release();
	p_fbo[i_fbo_idx]->release();
#endif	

	p_prg->bind();

#ifdef TEST_BUF_A
	channel = 0;
    TGles2Fns::glActiveTexture(GL_TEXTURE0 + channel);
	TGles2Fns::glBindTexture(GL_TEXTURE_2D, p_fbo[i_fbo_idx]->textureId());
#endif

	p_prg->setUniformValue("iTime", input.iTime);
	p_prg->setUniformValue("iFrame", input.iFrame);
	p_vao->bind();
	TGles2Fns::glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	p_vao->release();
#ifdef TEST_BUF_A
	TGles2Fns::glBindTexture(GL_TEXTURE_2D, 0);
#endif
	p_prg->release();
	
	SDL_GL_SwapWindow(wnd);
	SDL_GL_MakeCurrent(wnd, NULL);
	
	input.iTime += delta;
	input.iFrame++;
}

inline bool TMyApp::is_preview(void) const
{
	return is_screensaver && !is_fullscreen;
}

void TMyApp::init(bool is_screensaver, bool is_fullscreen, bool is_visible)
{
	is_running = true;
	this->is_screensaver = is_screensaver;
	this->is_fullscreen = is_fullscreen;
	render_flags = SDL_RENDERER_PRESENTVSYNC;

	input.iTime = 1.0f;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Не могу инициализировать видео драйвер: %s\n", SDL_GetError());
		exit(-1);
	}
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	const char caption[] = "alphawater";	

	if (is_fullscreen)
	{
	}
	else
	{
		width = 200;
		height = 200;

		wnd = SDL_CreateWindow(
			caption,
			width, height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
		
		if (!wnd)
		{
			SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Не могу создать окно: %s\n", SDL_GetError());
			exit(-1);
		}
	}

	input.iResolution = glm::vec3(width, height, 1.0f);
	
	ctx = SDL_GL_CreateContext(wnd);
	if (!ctx)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Не могу создать контекст: %s\n", SDL_GetError());
		exit(-1);
	}
	
	TGles2Fns::load();
	
    if (render_flags & SDL_RENDERER_PRESENTVSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }

	int status = SDL_GL_MakeCurrent(wnd, ctx);
	if (status)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
		exit(-1);
	}

	////////
	p_vao = new VertexArrayObject();
	p_vao->create();
	p_vao->bind();
	
    p_vbo_arr = new BufferObject(GL_ARRAY_BUFFER);
    p_vbo_arr->create();
    p_vbo_arr->bind();
    p_vbo_arr->setUsagePattern(GL_STATIC_DRAW);
    p_vbo_arr->allocate(vertices, sizeof(vertices));
	
	initilizeUniformValue();

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

    if (!p_prg->isLinked())
	{
        SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Не удалось слинковать шейдеры: %s", p_prg->log().c_str());
		exit(-1);
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
        SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Не удалось слинковать шейдеры (buf A): %s", p_prg_a->log().c_str());
		exit(-1);
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
    p_vbo_idx->allocate(indices, sizeof(indices));

	p_vao->release();
	//p_prg_a->release();

#ifdef TEST_BUF_A	
	p_fbo[0] = new FrameBuffer();
	p_fbo[0]->create(width, height, false);
	p_fbo[1] = new FrameBuffer();
	p_fbo[1]->create(width, height, false);
#endif
	SDL_GL_MakeCurrent(wnd, NULL);
/*

	if(!is_visible)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}

	const char caption[] = "alphawater";
	
	mon =  glfwGetPrimaryMonitor();
	if (is_fullscreen)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(mon);
		width = mode->width;
		height = mode->height;
		wnd = glfwCreateWindow(width, height, caption, mon, nullptr);

		glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		
		int dt = 100;
		wnd_pos[0] = dt;
		wnd_pos[1] = dt;
		dt <<= 1;
		wnd_size[0] = width - dt;
		wnd_size[1] = height - dt;
	}
	else
	{
		width = 200;
		height = 200;
		wnd = glfwCreateWindow(width, height, caption, nullptr, nullptr);

        glfwGetWindowSize(wnd, &wnd_size[0], &wnd_size[1]);
        glfwGetWindowPos(wnd, &wnd_pos[0], &wnd_pos[1]);
	}

    if (!wnd)
    {
        wcerr << L"failed to create wnd" << endl;
        exit(-1);
    }

	glfwSetWindowUserPointer(wnd, this);	
	{
		auto cb = [](GLFWwindow* wnd, int width, int height)
		{
			TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
			o->on_size(wnd, width, height);
		};
		glfwSetFramebufferSizeCallback(wnd, cb);
	}
	{
		auto cb = [](GLFWwindow* wnd, int key, int scancode, int action, int mods)
		{
			TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
			o->on_key(wnd, key, scancode, action, mods);
		};	
		glfwSetKeyCallback(wnd, cb);
	}

	if(is_screensaver)
	{
		{
			auto cb = [](GLFWwindow* wnd, double xpos, double ypos)
			{
				TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
				o->on_mouse_pos(wnd, xpos, ypos);
			};
			glfwSetCursorPosCallback(wnd, cb);
		}
		{
			auto cb = [](GLFWwindow* wnd, int button, int action, int mods)
			{
				TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
				o->on_mouse_btn(wnd, button, action, mods);
			};
			glfwSetMouseButtonCallback(wnd, cb);
		}
	}

    glfwMakeContextCurrent(wnd);
	
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        wcerr << L"failed to initialize glad with processes " << endl;
        exit(-1);
    }
	*/

    lastTime = SDL_GetTicks() * 0.001;
}

void TMyApp::show_usage(void)
{
	wcout << L"Неверные параметры!\n"
			L"Использование:\n"
			L"alphawater.exe [/c] | [/p <HWND>] | [/s]\n"
			L"\t/c             - показать диалог настроек\n"
			L"\t/p <HWND>      - предпросмотр в окне, дочернем по отношению к <HWND>\n"
			L"\t/s             - запуск в полноэкранном режиме\n\n"
			L"\tБез параметров - запуск в оконном режиме." << endl;
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
				/*HWND h_wnd = glfwGetWin32Window(wnd);
				SetParent(h_wnd, h_wnd_parent);
				SetWindowLong(h_wnd, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN);
				RECT rc;
				GetClientRect(h_wnd_parent, &rc);
				MoveWindow(h_wnd, rc.left, rc.top, rc.right, rc.bottom, TRUE);
				*/
			}
			else
			{
				//show_usage();
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
				//show_usage();
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
    delete p_vao;
    delete p_vbo_arr;
	delete p_vbo_idx;
	delete p_prg;
	SDL_DestroyWindow(wnd);
}

void TMyApp::run(void)
{
	SDL_Event e;

	while (is_running)
	{
		while (SDL_PollEvent(&e) && is_running)
		{
			switch (e.type)
			{
				case SDL_EVENT_WINDOW_RESIZED:
					on_size();
					break;

				case SDL_EVENT_QUIT:
					is_running = false;
					//break;
			}
		}
		draw();
	}
}

int main(int argc, char *argv[])
{
	int res;
	
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