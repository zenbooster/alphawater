#include <windows.h>
//#include <scrnsave.h>
#include <iostream>
#include <string>
#include <vector>
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

class TMyApp
{
	private:
		static const glm::vec2 screen;
		static const char caption[];
		static float quadVerts[];
		bool is_parent_console;
		uint32_t con_cp;
		bool is_fullscreen;
		bool is_screensaver;
		int i_mon_cnt;
		int i_wnd_cnt;
		GLFWmonitor** mon;
		int wnd_pos[2], wnd_size[2];
		
		//GLuint framebuffer;
		unsigned int shaderProgram;
		GLuint VAO;
		GLFWwindow** wnd;
		float *pf_time;
		float lastTime;
		
		string ConvertUTF8ToCp(const string& str);
		bool is_any_wnd_should_close();
		void init_wnd(GLFWwindow *wnd, int width, int height);
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
}; // class TMyApp

const glm::vec2 TMyApp::screen(1, 1);

const char TMyApp::caption[] = "alphawater";

float TMyApp::quadVerts[] = {
	-1.0, -1.0,     0.0, 0.0,
	-1.0, 1.0,      0.0, 1.0,
	1.0, -1.0,      1.0, 0.0,

	1.0, -1.0,      1.0, 0.0,
	-1.0, 1.0,      0.0, 1.0,
	1.0, 1.0,       1.0, 1.0
};

string TMyApp::ConvertUTF8ToCp(const string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, len);

    len = WideCharToMultiByte(con_cp, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* s = new char[len];
    WideCharToMultiByte(con_cp, 0, wstr, -1, s, len, NULL, NULL);

    std::string result(s);
    delete[] wstr;
    delete[] s;

    return result;
}

void TMyApp::init_wnd(GLFWwindow *wnd, int width, int height)
{
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
		cerr << "failed to initialize glad with processes" << endl;
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

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); 

	GLuint texColor;
	glGenTextures(1, &texColor);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////////
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
}

void TMyApp::set_mode(void)
{
    if (is_fullscreen)
    {
        // backup wnd position and wnd size
        glfwGetWindowPos(wnd[0], &wnd_pos[0], &wnd_pos[1] );
        glfwGetWindowSize(wnd[0], &wnd_size[0], &wnd_size[1] );

        // get resolution of monitor
        const GLFWvidmode * mode = glfwGetVideoMode(mon[0]);
        // switch to full screen
        glfwSetWindowMonitor(wnd[0], mon[0], 0, 0, mode->width, mode->height, 0);
		glfwSetInputMode(wnd[0], GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		i_wnd_cnt = i_mon_cnt;
		for(int i = 1; i < i_wnd_cnt; i++)
		{
			int width;
			int height;

			mode = glfwGetVideoMode(mon[i]);
			width = mode->width;
			height = mode->height;

			wnd[i] = glfwCreateWindow(width, height, caption, mon[i], nullptr);
			if (!wnd[i])
			{
				cerr << "failed to create wnd" << endl;
				exit(-1);
			}

			glfwSetInputMode(wnd[i], GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			init_wnd(wnd[i], width, height);
		}
    }
    else
    {
		for(int i = 1; i < i_wnd_cnt; i++)
		{
			glfwDestroyWindow(wnd[i]);
			wnd[i] = NULL;
		}

		i_wnd_cnt = 1;
        // restore last wnd size and position
        glfwSetWindowMonitor(wnd[0], nullptr,  wnd_pos[0], wnd_pos[1], wnd_size[0], wnd_size[1], 0);

		glfwSetInputMode(wnd[0], GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
	float now = glfwGetTime();
	float delta = now - lastTime;

	lastTime = now;
	
	for(int i = 0; i < i_wnd_cnt; i++)
	{
		glfwMakeContextCurrent(wnd[i]);
		pf_time[i] += delta;

		glUseProgram(shaderProgram);
		glUniform1f(glGetUniformLocation(shaderProgram, "fTime"), pf_time[i]);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(wnd[i]);
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
	wnd = new GLFWwindow * [i_mon_cnt];
	pf_time = new float[i_mon_cnt];
	for(int i = 0; i < i_mon_cnt; i++)
	{
		pf_time[i] = rand() % 1000;
	}
	
	if (is_fullscreen)
	{
		i_wnd_cnt = i_mon_cnt;
		for(int i = i_wnd_cnt - 1; i >= 0; i--)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(mon[i]);
			width = mode->width;
			height = mode->height;
			cout << "mon[" << i << "] = " << mon[i] << endl;
			wnd[i] = glfwCreateWindow(width, height, caption, mon[i], nullptr);
			if (!wnd[i])
			{
				cerr << "failed to create wnd" << endl;
				exit(-1);
			}

			glfwSetInputMode(wnd[i], GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
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
			//wnd[i] = glfwCreateWindow(width, height, caption, nullptr, nullptr);
			wnd[i] = glfwCreateWindow(width, height, cap.c_str(), nullptr, nullptr);
			if (!wnd[i])
			{
				cerr << "failed to create wnd" << endl;
				exit(-1);
			}
		}

        glfwGetWindowSize(wnd[0], &wnd_size[0], &wnd_size[1]);
        glfwGetWindowPos(wnd[0], &wnd_pos[0], &wnd_pos[1]);
	}

	for(int i = i_wnd_cnt - 1; i >= 0; i--)
	{
		//MessageBoxA(NULL, "debug", "HIT.1", MB_OK);
		init_wnd(wnd[i], width, height);

		lastTime = glfwGetTime();
		
		//glfwMaximizeWindow(wnd[i]);
	}
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
			
	cout << ConvertUTF8ToCp(s) << endl;
}

TMyApp::TMyApp(int argc, char *argv[])
{
	is_parent_console = AttachConsole(ATTACH_PARENT_PROCESS);

	if(is_parent_console)
	{
		freopen("CON", "w", stdout);
		freopen("CON", "r", stdin);
		freopen("CON", "w", stderr);
		cout << endl;
		
		con_cp = GetConsoleCP();
	}

	switch(argc)
	{
		case 1 + 2:
		{
			if(!strcmp(argv[1], "/p"))
			{
				init(true, false, false);

				char *s_ptr = argv[2];
				HWND h_wnd_parent = (HWND)stoull(s_ptr, nullptr, 10);
				HWND h_wnd = glfwGetWin32Window(wnd[0]);
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
	cout << "exit" << endl;
	
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
		if(glfwWindowShouldClose(wnd[i]))
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