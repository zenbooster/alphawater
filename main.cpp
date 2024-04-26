#include <windows.h>
//#include <scrnsave.h>
#include <iostream>
#include <string>
#include <vector>
//#include <io.h>
//#include <fcntl.h>

#include "common.h"
#include "TMyApp.h"

using namespace std;

int main(int argc, char *argv[])
{
	int res;

    log4cplus::Initializer initializer;

    //log4cplus::BasicConfigurator config;
    //config.configure();
	log4cplus::SharedAppenderPtr fileAppender(new log4cplus::FileAppender(LOG4CPLUS_TEXT("./main.log"),std::ios_base::app,true,true));
	fileAppender->setName(LOG4CPLUS_TEXT("file"));
	log4cplus::tstring pattern = LOG4CPLUS_TEXT("%D{%Y/%m/%d %H:%M:%S,%Q} [%t] %-5p %c - %m [%l]%n");
    fileAppender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));
	log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
	logger.addAppender(fileAppender);
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("vid"));
	logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
	logger.addAppender(fileAppender);
	
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT ("app"));
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("start"));

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