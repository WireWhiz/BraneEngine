//
// Created by eli on 7/9/2022.
//

#ifndef BRANEENGINE_LOGGING_H
#define BRANEENGINE_LOGGING_H

#include <string>
#include <vector>
#include <ctime>
#include <functional>

namespace Logging
{
	enum class LogLevel
	{
		error = 0,
		warning = 1,
		log = 2,
		verbose = 3
	};

	enum class LogColor
	{
		reset,
		white,
		red,
		yellow
	};

	struct Log
	{
		std::string message;
		LogLevel level;
		time_t time;
		std::string toString() const;
	};

	extern bool printToConsole;

	void init();
	void cleanup();
	void pushLog(std::string message, LogLevel level);
	void addListener(std::function<void(const Log&)> callback);
	void setColor(LogColor color);
};


#endif //BRANEENGINE_LOGGING_H
