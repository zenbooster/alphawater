#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

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
  vec4 v = vec4(0, 33, 55,0);
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

typedef struct GLES2_Context
{
#define SDL_PROC(ret, func, params) ret (APIENTRY *func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC
} GLES2_Context;

static int LoadContext(GLES2_Context *data)
{
#ifdef SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif defined(SDL_VIDEO_DRIVER_ANDROID)
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define SDL_PROC(ret, func, params) data->func = func;
#else
#define SDL_PROC(ret, func, params)                                                            \
    do {                                                                                       \
        data->func = (ret (APIENTRY *) params)SDL_GL_GetProcAddress(#func);                    \
        if (!data->func) {                                                                     \
            return SDL_SetError("Couldn't load GLES2 function %s: %s", #func, SDL_GetError()); \
        }                                                                                      \
    } while (0);
#endif /* __SDL_NOGETPROCADDR__ */

#include "SDL_gles2funcs.h"
#undef SDL_PROC
    return 0;
}

typedef struct shader_data
{
    GLuint shader_program, shader_frag, shader_vert;

    GLint attr_position;
    //GLint attr_color, attr_mvp;
	GLint attr_iResolution, attr_fTime;

    int angle_x, angle_y, angle_z;

    GLuint position_buffer;
    GLuint color_buffer;
} shader_data;

GLES2_Context gles2_ctx;

#define GL_CHECK(x)                                                                         \
    x;                                                                                      \
    {                                                                                       \
        GLenum glError = gles2_ctx.glGetError();                                                  \
        if (glError != GL_NO_ERROR) {                                                       \
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "glGetError() = %i (0x%.8x) at line %i\n", glError, glError, __LINE__); \
            exit(-1);                                                                        \
        }                                                                                   \
    }

static void
process_shader(GLuint *shader, const char *source, GLint shader_type)
{
    GLint status = GL_FALSE;
    const char *shaders[1] = { NULL };
    char buffer[1024];
    GLsizei length = 0;

    /* Create shader and load into GL. */
    *shader = GL_CHECK(gles2_ctx.glCreateShader(shader_type));

    shaders[0] = source;

    GL_CHECK(gles2_ctx.glShaderSource(*shader, 1, shaders, NULL));

    /* Clean up shader source. */
    shaders[0] = NULL;

    /* Try compiling the shader. */
    GL_CHECK(gles2_ctx.glCompileShader(*shader));
    GL_CHECK(gles2_ctx.glGetShaderiv(*shader, GL_COMPILE_STATUS, &status));

    /* Dump debug info (source and log) if compilation failed. */
    if (status != GL_TRUE) {
        gles2_ctx.glGetShaderInfoLog(*shader, sizeof(buffer), &length, &buffer[0]);
        buffer[length] = '\0';
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Shader compilation failed: %s", buffer);
        exit(-1);
    }
}

static void
link_program(struct shader_data *data)
{
    GLint status = GL_FALSE;
    char buffer[1024];
    GLsizei length = 0;

    GL_CHECK(gles2_ctx.glAttachShader(data->shader_program, data->shader_vert));
    GL_CHECK(gles2_ctx.glAttachShader(data->shader_program, data->shader_frag));
    GL_CHECK(gles2_ctx.glLinkProgram(data->shader_program));
    GL_CHECK(gles2_ctx.glGetProgramiv(data->shader_program, GL_LINK_STATUS, &status));

    if (status != GL_TRUE) {
        gles2_ctx.glGetProgramInfoLog(data->shader_program, sizeof(buffer), &length, &buffer[0]);
        buffer[length] = '\0';
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Program linking failed: %s", buffer);
        exit(-1);
    }
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
		//unsigned int shaderProgram;
		GLuint VAO;
		SDL_Window* wnd;
		SDL_GLContext ctx;
		shader_data data;
		float f_time;
		float lastTime;

		//void set_mode(void);
		//void on_size(GLFWwindow* wnd, int width, int height);
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

/*void TMyApp::on_size(__attribute__((unused)) GLFWwindow* wnd, int width, int height)
{
    glViewport(0, 0, width, height);
    draw();
}

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

	GL_CHECK(gles2_ctx.glUniform1f(data.attr_fTime, f_time));
	gles2_ctx.glBindVertexArray(VAO);
	gles2_ctx.glDrawArrays(GL_TRIANGLES, 0, 6);
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

	GLfloat screen[2] = {1, 1};

	f_time = 1.0f;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Не могу инициализировать видео драйвер: %s\n", SDL_GetError());
		exit(-1);
	}
	
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
	
	if (LoadContext(&gles2_ctx) < 0)
	{
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Не могу загрузить GLES2 функции.\n");
        exit(-1);
    }
	
	int render_flags = SDL_RENDERER_PRESENTVSYNC;

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

	//gles2_ctx.glViewport(0, 0, width, height);

	////////
    GL_CHECK(gles2_ctx.glGenVertexArrays(1, &VAO));
	SDL_Log("HIT.3\n");	
    GL_CHECK(gles2_ctx.glBindVertexArray(VAO));

    GLuint VBO;
    GL_CHECK(gles2_ctx.glGenBuffers(1, &VBO));
    GL_CHECK(gles2_ctx.glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(gles2_ctx.glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW));

    GL_CHECK(gles2_ctx.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0)));
    GL_CHECK(gles2_ctx.glEnableVertexAttribArray(0));

    GL_CHECK(gles2_ctx.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float))));
    GL_CHECK(gles2_ctx.glEnableVertexAttribArray(1));

    GL_CHECK(gles2_ctx.glBindVertexArray(0));

    GL_CHECK(gles2_ctx.glGenFramebuffers(1, &framebuffer));
    GL_CHECK(gles2_ctx.glBindFramebuffer(GL_FRAMEBUFFER, framebuffer)); 

    GLuint texColor;
    GL_CHECK(gles2_ctx.glGenTextures(1, &texColor));
    GL_CHECK(gles2_ctx.glBindTexture(GL_TEXTURE_2D, texColor));
    GL_CHECK(gles2_ctx.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));
    GL_CHECK(gles2_ctx.glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(gles2_ctx.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0));

    GL_CHECK(gles2_ctx.glBindFramebuffer(GL_FRAMEBUFFER, 0));
	

	////////

	data.angle_x = 0;
	data.angle_y = 0;
	data.angle_z = 0;

	process_shader(&data.shader_vert, vertexShaderSource, GL_VERTEX_SHADER);
	process_shader(&data.shader_frag, fragmentShaderSource, GL_FRAGMENT_SHADER);
	
	data.shader_program = GL_CHECK(gles2_ctx.glCreateProgram());

	link_program(&data);
	
    data.attr_iResolution = GL_CHECK(gles2_ctx.glGetUniformLocation(data.shader_program, "iResolution"));
    data.attr_fTime = GL_CHECK(gles2_ctx.glGetUniformLocation(data.shader_program, "fTime"));

	GL_CHECK(gles2_ctx.glUseProgram(data.shader_program));
	GL_CHECK(gles2_ctx.glUniform2fv(data.attr_iResolution, 1, screen));

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
	SDL_DestroyWindow(wnd);
}

void TMyApp::run(void)
{
	SDL_Event e;

	while (is_running)
	{
		while (SDL_PollEvent(&e) && is_running)
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				is_running = false;
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