//
// Created by eli on 7/9/2022.
//

#include "logging.h"

#include <fstream>
#include <config/config.h>
#include <filesystem>
#include <string>

#if WIN32
#include <windows.h>
#endif

namespace Logging
{
	bool printToConsole = true;
	std::ofstream _logFile;
	std::vector<std::function<void(const Log&)>> _logListeners;
#if WIN32
	HANDLE hConsole;
#endif

    tm ltm_global = tm();
    tm& getLocalTime()
    {
        const time_t now = time(0);
#if WIN32
		localtime_s(&ltm_global, &now);
#else
        localtime_r(&now, &ltm_global);
#endif
        return ltm_global;
    }

	std::string generateLogName(){
		tm& ltm = getLocalTime();
		std::string date = std::to_string(ltm.tm_year + 1900) + "_" + std::to_string(ltm.tm_mon + 1) + "_" + std::to_string(ltm.tm_mday) + "_" + std::to_string(ltm.tm_min);
		return "log_" + date + ".txt";
	}

	void init()
	{
		std::string path = Config::json().get("logs_directory", "temp/logs").asString();
		std::filesystem::create_directories(path);
		_logFile = std::ofstream(path + "/" + generateLogName(), std::ios::binary);
		if(!_logFile.is_open())
			throw std::runtime_error("Could not generate log file");

#if WIN32
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	}

	void cleanup()
	{
		if(_logFile.is_open())
			_logFile.close();
	}

	void pushLog(std::string message, LogLevel level)
	{
		Log newLog{message, level, time(0)};
		std::string fStr = newLog.toString();
		_logFile << fStr << std::endl;
		for(auto& f : _logListeners)
		{
			try{
				f(newLog);
			}
			catch(const std::exception& e)
			{
				std::cerr << "Log callback threw error!" << std::endl;
			}
		}
		if(printToConsole)
		{
			switch(level)
			{
				case LogLevel::verbose:
				case LogLevel::log:
					std::cout << fStr << std::endl;
					break;
				case LogLevel::warning:
					setColor(LogColor::yellow);
					std::cout << fStr << std::endl;
					setColor(LogColor::reset);
					break;
				case LogLevel::error:
					setColor(LogColor::red);
					std::cerr << fStr << std::endl;
					setColor(LogColor::reset);
			}


		}
	}

	void addListener(std::function<void(const Log&)> callback)
	{
		_logListeners.push_back(callback);
	}

	std::string Log::toString() const
	{
		tm& ltm = getLocalTime();
		std::string timeStr = std::to_string(ltm.tm_hour) + ":" + std::to_string(ltm.tm_min) + ":" + std::to_string(ltm.tm_sec);

		std::string levelStr;
		switch(level)
		{
			case LogLevel::error:
				levelStr = "ERROR";
				break;
			case LogLevel::warning:
				levelStr = "WARNING";
				break;
			case LogLevel::log:
				levelStr = "LOG";
				break;
			case LogLevel::verbose:
				levelStr = "VERBOSE";
				break;
		}
		return "[" + timeStr + "][" + levelStr + "]: " + message;
	}

	void setColor(LogColor color)
	{
		switch(color)
		{
			case LogColor::reset:
				std::cout << "\033[0m";
				break;
			case LogColor::white:
				std::cout << "\033[37m";
				break;
			case LogColor::red:
				std::cout << "\033[31m";
				break;
			case LogColor::yellow:
				std::cout << "\033[33m";
				break;
		}
	}
}

