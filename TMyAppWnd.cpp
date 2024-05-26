#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#pragma GCC diagnostic pop
#include "TMyAppWnd.h"
#include "TMyApp.h"
#include "common.h"
#include <unordered_map>

extern string s_pack_name;

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
	
	bool is_common = m_file_content.count(ef_common);
	
	map<TEnumFile, TMPShader *> m_mpshaders;
	for(auto v : m_file_content)
	{
		TEnumFile ef = v.first;
		string fsh_src = v.second;
		
		if(is_common)
		{
			fsh_src = m_file_content[ef_common] + "\n" + fsh_src;
		}
		m_mpshaders[ef] = new TMPShader(this, m_file_names[ef], vertexShader, fsh_src.c_str());
	}
	
	map<string, TEnumFile> m_tok2ef = {
		{"i", ef_image},
		{"a", ef_buf_a},
		{"b", ef_buf_b},
		{"c", ef_buf_c},
		{"d", ef_buf_d}
	};
	
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
		ifstream f(fspec);
		string line;
		if(getline(f, line))
		{
			vector<string> tokens;
			int i_tokens_size;

			split(line, ',', tokens);
			i_tokens_size = (int)tokens.size();

			if(i_tokens_size != 5)
			{
				LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Неверное количество столбцов в заголовке: ") << line);
				throw exception();
			}
			
			if(	tokens[0] != "prg" || 
				tokens[1] != "ch0" ||
				tokens[2] != "ch1" ||
				tokens[3] != "ch2" ||
				tokens[4] != "ch3"
			)
			{
				LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Неверный заголовок: ") << line);
				throw exception();
			}

			string prg;//, ch0, ch1, ch2, ch3;
			map<int, string> m_ch;
			
			struct TAssignInfo
			{
				TMPShader *src;
				int ch;
				TMPShader *dst;
				TTexParam par;
			};
			vector<TAssignInfo> assign_info;
			
			unordered_map<TMPShader *, TTexParams> ef2tp;
			
			while(getline(f, line))
			{
				tokens.clear();
				split(line, ',', tokens);
				i_tokens_size = (int)tokens.size();

				if(i_tokens_size < 1 || i_tokens_size > 5)
				{
					LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Неверное количество столбцов в данных: ") << line);
					throw exception();
				}

				prg = tokens[0];

				stringstream ss_log;
				ss_log << prg << ": ";
				TEnumFile ef = m_tok2ef[prg];
				
				for(int i = 1; i < i_tokens_size; i++)
				{
					LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("loop begin"));
					int ch = i - 1;
					string tgt = tokens[i];
					
					ss_log << tgt;
					if(ch < i_tokens_size - 2)
					{
						ss_log << ", ";
					}

					if(!tgt.empty())
					{
						vector<string> subtokens;
						int i_subtokens_size;

						LOG4CPLUS_ERROR(logger, "tgt: " << tgt);
						split(tgt, ':', subtokens);
						i_subtokens_size = (int)subtokens.size();
						LOG4CPLUS_ERROR(logger, "i_subtokens_size: " << i_subtokens_size);
						
						tgt = subtokens[0];
						
						TTexParam par;
						GLenum *p[] = {&par.filterMode, &par.wrapMode};
						unordered_map<string, GLenum> c2fm = {
							{"n", GL_NEAREST},
							{"l", GL_LINEAR},
							{"m", GL_MIPMAP},
							{"", GL_LINEAR}
						};
						unordered_map<string, GLenum> c2wm = {
							{"c", GL_CLAMP},
							{"r", GL_REPEAT},
							{"", GL_CLAMP}
						};
						vector<unordered_map<string, GLenum>*> c2x = {&c2fm, &c2wm};
						
						for(int k = 0, j = 1; j < i_subtokens_size; k++, j++)
						{
							LOG4CPLUS_ERROR(logger, "subtokens[" << j << "]: " << subtokens[j]);
							*p[k] = (*c2x[k])[subtokens[j]];
						}

						TEnumFile ef_tgt = m_tok2ef[tgt];
						TAssignInfo ai = {m_mpshaders[ef], ch, m_mpshaders[ef_tgt], par};
						assign_info.push_back(ai);
						
						ef2tp[ai.dst].insert(par);
					}
					LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("loop end"));
				}
				LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("") << ss_log.str());
			} // while(getline(f, line))

			for(auto& v : assign_info)
			{
				v.src->assign(v.ch, v.dst, v.par, ef2tp[v.dst]);
			}
		}
	}
	mpsh = m_mpshaders[ef_image];
	mpsh->link();
	
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Пакет загружен успешно!"));
}

void TMyAppWnd::init_wnd(int width, int height)
{
	this->width = width;
	this->height = height;
	
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
	
	glfwSwapInterval( 1 );

	p_vao = new VertexArrayObject();
	p_vao->create();
	p_vao->bind();
	
    p_vbo_arr = new BufferObject(GL_ARRAY_BUFFER);
    p_vbo_arr->create();
    p_vbo_arr->bind();
    p_vbo_arr->setUsagePattern(GL_STATIC_DRAW);
    p_vbo_arr->allocate(p_app->vertices, sizeof(p_app->vertices));
	
	initilizeUniformValue(width, height);
	
	load(s_pack_name);

    p_vbo_idx = new BufferObject(GL_ELEMENT_ARRAY_BUFFER);
    p_vbo_idx->create();
    p_vbo_idx->setUsagePattern(GL_STATIC_DRAW);
    p_vbo_idx->bind();
    p_vbo_idx->allocate(p_app->indices, sizeof(p_app->indices));

	p_vao->release();
}

void TMyAppWnd::on_size(int width, int height)
{
	this->width = width;
	this->height = height;

	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("wnd=") << wnd << LOG4CPLUS_TEXT("; w=") << width << LOG4CPLUS_TEXT("; h=") << height);

	glfwMakeContextCurrent(wnd);
	int t = max(width, height);
	glViewport(-(t-width)*0.5, -(t-height)*0.5, t, t);

	input.iResolution = glm::vec3(width, height, 1.0f);
	mpsh->resize(width, height);

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
	mpsh->draw();

	glfwMakeContextCurrent(NULL);
	glfwSwapBuffers(wnd);

	input.iTimeDelta = p_app->delta;
	input.iTime += p_app->delta;
	input.iFrame++;
}
