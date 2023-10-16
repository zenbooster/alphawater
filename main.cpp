#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    //mainImage(color, fragmentTexCoord);
	mainImage(color, texCoord);
    fragColor = color;
})";

class TMyApp
{
	private:
		static float quadVerts[];
	    bool is_fullscreen;
		GLFWmonitor* mon;
		int _wndPos[2], _wndSize[2];
		
		GLuint framebuffer;
		unsigned int shaderProgram;
		GLuint VAO;
		GLFWwindow* window;
		float f_time;
		float lastTime;

		void set_mode(void);
		void on_size(GLFWwindow* window, int width, int height);
		void draw(void);
	public:
		TMyApp();
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
        // backup window position and window size
        glfwGetWindowPos(window, &_wndPos[0], &_wndPos[1] );
        glfwGetWindowSize(window, &_wndSize[0], &_wndSize[1] );
        
        // get resolution of monitor
        const GLFWvidmode * mode = glfwGetVideoMode(mon);

        // switch to full screen
        glfwSetWindowMonitor(window, mon, 0, 0, mode->width, mode->height, 0);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        // restore last window size and position
        glfwSetWindowMonitor(window, nullptr,  _wndPos[0], _wndPos[1], _wndSize[0], _wndSize[1], 0 );

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void TMyApp::on_size(__attribute__((unused)) GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    draw();
}

void TMyApp::draw(void)
{
	float now = glfwGetTime();
	float delta = now - lastTime;

	lastTime = now;
	f_time += delta;

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(shaderProgram);
	glUniform1f(glGetUniformLocation(shaderProgram, "fTime"), f_time); 
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(window);
}

TMyApp::TMyApp():
	//is_fullscreen(true)
	is_fullscreen(false)
{
    int width;
    int height;

	glm::vec2 screen(1, 1);

	f_time = 1.0f;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	const char caption[] = "alphawater";
	
	mon =  glfwGetPrimaryMonitor();
	if (is_fullscreen)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(mon);
		width = mode->width;
		height = mode->height;
		window = glfwCreateWindow(width, height, caption, mon, nullptr);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		
		int dt = 100;
		_wndPos[0] = dt;
		_wndPos[1] = dt;
		dt <<= 1;
		_wndSize[0] = width - dt;
		_wndSize[1] = height - dt;
	}
	else
	{
		width = 200;
		height = 200;
		window = glfwCreateWindow(width, height, caption, nullptr, nullptr);

        glfwGetWindowSize(window, &_wndSize[0], &_wndSize[1]);
        glfwGetWindowPos(window, &_wndPos[0], &_wndPos[1]);
	}

    if (!window)
    {
        cerr << "failed to create window" << endl;
        exit(-1);
    }

	glfwSetWindowUserPointer(window, this);	

	auto cb = [](GLFWwindow* window, int width, int height)
	{
		TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(window));
		o->on_size(window, width, height);
	};
	glfwSetFramebufferSizeCallback(window, cb);

    glfwMakeContextCurrent(window);
	
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "failed to initialize glad with processes " << endl;
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

TMyApp::~TMyApp()
{
    glfwTerminate();
    // cleanup
}

void TMyApp::run(void)
{
	bool is_mode_switch = false;
	
    while (!glfwWindowShouldClose(window))
    {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		if ((glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) &&
			(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS))
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


		draw();
        glfwPollEvents();
    }
}

int main()
{
	TMyApp app;
	app.run();
}