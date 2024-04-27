#pragma once
#include "shaderprogram.h"
#include "TMPShader.h"
#include "shaders.h"
#include "shaderinput.h"
#include "vertexarrayobject.h"
#include "bufferobject.h"
#include "framebuffer.h"

class TMyApp;

class TMyAppWnd
{
	friend class TMyApp;
	friend class TMPShader;

private:
	TMyApp *p_app;
	GLFWwindow *wnd;
	int width;
	int height;
	ShaderInput input;

	TMPShader *mpsh;

	VertexArrayObject *p_vao;
	BufferObject *p_vbo_arr;
	BufferObject *p_vbo_idx;

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