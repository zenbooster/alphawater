#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "TGles2Fns.h"
#include "shaderprogram.h"

using namespace std;

const char *vertexShaderSource = 
R"(#version 330 core
layout (location = 0) in vec2 position;            
layout (location = 1) in vec2 inTexCoord;

out vec2 texCoord;
void main(){
    texCoord = inTexCoord;
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
})";

const char *fragmentShaderSource = 
R"(#version 330 core
in vec2 texCoord;
uniform vec2      iResolution;
uniform float     fTime;
out vec4 fragColor;

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
    float t = mod(fTime, 251.328);
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

void main() {
    vec4 color;
	mainImage(color, texCoord);
    fragColor = color;
})";

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
		static float quadVerts[];
		bool is_running;
	    bool is_fullscreen;
		bool is_screensaver;
		//GLFWmonitor* mon;
		int wnd_pos[2], wnd_size[2];
		
		GLuint framebuffer;
		GLuint VAO;
		SDL_Window* wnd;
		SDL_GLContext ctx;
		int render_flags;
		ShaderProgram *p_prg;
		float f_time;
		float lastTime;

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

float TMyApp::quadVerts[] = {
	-1.0, -1.0,     0.0, 0.0,
	-1.0, 1.0,      0.0, 1.0,
	1.0, -1.0,      1.0, 0.0,

	1.0, -1.0,      1.0, 0.0,
	-1.0, 1.0,      0.0, 1.0,
	1.0, 1.0,       1.0, 1.0
};

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
	int w, h;

	SDL_GetWindowSize(wnd, &w, &h);
	
	int status = SDL_GL_MakeCurrent(wnd, ctx);
	if (status)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
		exit(-1);
	}

    TGles2Fns::glViewport(0, 0, w, h);
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
	f_time += delta;

	int status = SDL_GL_MakeCurrent(wnd, ctx);
	if (status)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
		exit(-1);
	}

	p_prg->bind();
	p_prg->setUniformValue("fTime", f_time);
	TGles2Fns::glBindVertexArray(VAO);
	TGles2Fns::glDrawArrays(GL_TRIANGLES, 0, 6);
	//
	SDL_GL_SwapWindow(wnd);
	SDL_GL_MakeCurrent(wnd, NULL);
}

inline bool TMyApp::is_preview(void) const
{
	return is_screensaver && !is_fullscreen;
}

void TMyApp::init(bool is_screensaver, bool is_fullscreen, bool is_visible)
{
    int width;
    int height;
	
	is_running = true;
	this->is_screensaver = is_screensaver;
	this->is_fullscreen = is_fullscreen;
	render_flags = SDL_RENDERER_PRESENTVSYNC;

	glm::vec2 screen(1, 1);

	f_time = 1.0f;

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

	//TGles2Fns::glViewport(0, 0, width, height);

	////////
    GL_CHECK(TGles2Fns::glGenVertexArrays(1, &VAO));
    GL_CHECK(TGles2Fns::glBindVertexArray(VAO));

    GLuint VBO;
    GL_CHECK(TGles2Fns::glGenBuffers(1, &VBO));
    GL_CHECK(TGles2Fns::glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(TGles2Fns::glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW));

    GL_CHECK(TGles2Fns::glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0)));
    GL_CHECK(TGles2Fns::glEnableVertexAttribArray(0));

    GL_CHECK(TGles2Fns::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float))));
    GL_CHECK(TGles2Fns::glEnableVertexAttribArray(1));

    GL_CHECK(TGles2Fns::glBindVertexArray(0));

    GL_CHECK(TGles2Fns::glGenFramebuffers(1, &framebuffer));
    GL_CHECK(TGles2Fns::glBindFramebuffer(GL_FRAMEBUFFER, framebuffer)); 

    GLuint texColor;
    GL_CHECK(TGles2Fns::glGenTextures(1, &texColor));
    GL_CHECK(TGles2Fns::glBindTexture(GL_TEXTURE_2D, texColor));
    GL_CHECK(TGles2Fns::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));
    GL_CHECK(TGles2Fns::glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(TGles2Fns::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0));

    GL_CHECK(TGles2Fns::glBindFramebuffer(GL_FRAMEBUFFER, 0));
	

	////////
	//process_shader(&data.shader_vert, vertexShaderSource, GL_VERTEX_SHADER);
	//process_shader(&data.shader_frag, fragmentShaderSource, GL_FRAGMENT_SHADER);
	
	//data.shader_program = GL_CHECK(TGles2Fns::glCreateProgram());

	//link_program(&data);
	
	p_prg = new ShaderProgram();
	p_prg->addShaderFromSource(Shader::ShaderType::Vertex, vertexShaderSource);
	p_prg->addShaderFromSource(Shader::ShaderType::Fragment, fragmentShaderSource);
    p_prg->link();

    if (!p_prg->isLinked())
	{
        SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Не удалось слинковать шейдеры: %s", p_prg->log().c_str());
		exit(-1);
	}
	
    //data.attr_iResolution = GL_CHECK(TGles2Fns::glGetUniformLocation(data.shader_program, "iResolution"));
    //data.attr_fTime = GL_CHECK(TGles2Fns::glGetUniformLocation(data.shader_program, "fTime"));
	p_prg->bind();
	p_prg->setUniformValue("iResolution", screen);

	//GL_CHECK(TGles2Fns::glUseProgram(data.shader_program));
	//GL_CHECK(TGles2Fns::glUniform2fv(data.attr_iResolution, 1, screen));

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