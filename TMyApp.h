#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

#include "TMyAppWnd.h"
#include "common.h"


class TMyAppWnd;

class TMyApp
{
	friend class TMyAppWnd;

	private:
		static const glm::vec2 screen;
		constexpr static char caption[] = "alphawater";

		constexpr static GLfloat vertices[] = {
		   -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		   -1.0f, -1.0f,  0.0f,  0.0f,  0.0f
		};

		constexpr static GLuint indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		
		log4cplus::Logger logger;
		
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
		bool is_preview(void) const;
		void init(bool is_screensaver, bool is_fullscreen, bool is_visible);
		void show_usage(void);

	public:
		//TMyApp(bool is_visible = true);
		TMyApp(int argc, char *argv[]);
		~TMyApp();
		
		void run(void);
}; // class TMyApp