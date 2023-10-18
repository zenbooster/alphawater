#include <limits>
#include <lib/fstrcmp.h>

#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

#include <GL/gl.h>

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
    float t = fTime;
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

class TMyApp
{
	private:
		static float quadVerts[];
	    bool is_fullscreen;
		bool is_screensaver;
		GLFWmonitor* mon;
		int wnd_pos[2], wnd_size[2];
		
		GLuint framebuffer;
		unsigned int shaderProgram;
		GLuint VAO;
		GLFWwindow* wnd;
		float f_time;
		float lastTime;

		void set_mode(void);
		void on_size(GLFWwindow* wnd, int width, int height);
		void on_key(GLFWwindow* wnd, int key, int scancode, int action, int mods);
		void on_mouse_pos(GLFWwindow* wnd, double xpos, double ypos);
		void on_mouse_btn(GLFWwindow* wnd, int button, int action, int mods);
		void draw(void);
		inline bool is_preview(void) const;
		void init(bool is_screensaver, bool is_fullscreen, bool is_visible);
		void show_usage(void);

	public:
		//TMyApp(bool is_visible = true);
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

void TMyApp::set_mode(void)
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
}

void TMyApp::on_size(__attribute__((unused)) GLFWwindow* wnd, int width, int height)
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
}

void TMyApp::draw(void)
{
	static size_t sz = 200 * 200 * 4;
	static uint8_t *p_first = new uint8_t[sz];
	static uint8_t *p_next = new uint8_t[sz];
	static bool is_first = true;
	static int i_first = 10000;
	static int res = std::numeric_limits<int>::max();
	//static float f_old_time = f_time;
	static double res2 = 0.0;
	float now = glfwGetTime();
	float delta = 0.001;//now - lastTime;

	lastTime = now;
	f_time += delta;

	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(shaderProgram);
	glUniform1f(glGetUniformLocation(shaderProgram, "fTime"), f_time); 
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	cout << "HIT.1" << endl;
	
	if (is_first)
	{
		glReadPixels(0, 0, 200, 200, GL_RGBA, GL_UNSIGNED_BYTE, p_first);
		is_first = false;
		//f_time = 63.8;
		f_time = 252.0;
		delta = 0.0000001;
	}
	/*else
	if(i_first)
	{
		i_first--;
	}*/
	else
	{
		glReadPixels(0, 0, 200, 200, GL_RGBA, GL_UNSIGNED_BYTE, p_next);
		int r = abs(memcmp(p_first, p_next, sz));

		if (r < res)
		{
			wcout << L"r = " << r << L"; res = " << res << L"; f_time = " << f_time << endl;
			res = r;
			if(!r)
			{
				glfwSetWindowShouldClose(wnd, true);
			}
		}
		
		/*if (f_time - f_old_time > 10)
		{
			f_old_time = f_time;
			wcout << L"f_time = " << f_time << endl;
		}*/

		int ofs = 99 * 200;
		int r2 = fmemcmpi(p_first+ofs, 200*2*4, p_next+ofs, 200*2*4);
		if (r2 > res2)
		{
			wcout << L"r2 = " << r2 << L"; res2 = " << res2 << L"; f_time = " << f_time;
			if (r2 == FSTRCMP_IDENTICAL)
			{
				wcout << L" IDENTICAL !";
				glfwSetWindowShouldClose(wnd, true);
			}
			else
			if (r2 > FSTRCMP_THRESHOLD)
			{
				wcout << L" SIMILAR !";
			}
			wcout << endl;

			res2 = r2;
		}
	}

	glfwSwapBuffers(wnd);
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

	glm::vec2 screen(1, 1);

	f_time = 1.0f;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

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

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); 

    GLuint texColor;
    glGenTextures(1, &texColor);
    glBindTexture(GL_TEXTURE_2D, texColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
	glUniform2fv(glGetUniformLocation(shaderProgram, "iResolution"), 1, &screen[0]);

    lastTime = glfwGetTime();
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
				HWND h_wnd = glfwGetWin32Window(wnd);
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
    glfwTerminate();
    // cleanup
}

void TMyApp::run(void)
{
    while (!glfwWindowShouldClose(wnd))
    {
		draw();
        glfwPollEvents();
    }
}

int main(int argc, char *argv[])
{
	int res;
	
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
	
	wcout << L"HIT.0" << endl;

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