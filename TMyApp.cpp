#include "windows.h"
#include "TMyApp.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

const glm::vec2 TMyApp::screen(1, 1);

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

bool TMyApp::is_preview(void) const
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
			if(i & 1)
			{
				wnd[i]->input.iTimeDelta = -wnd[i]->input.iTimeDelta;
			}
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
