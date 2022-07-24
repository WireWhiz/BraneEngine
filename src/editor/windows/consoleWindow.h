//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_CONSOLEWINDOW_H
#define BRANEENGINE_CONSOLEWINDOW_H

#include <ui/guiWindow.h>
#include <string>
#include <runtime/logging.h>

class ConsoleWindow : public GUIWindow
{
	struct CachedLog
	{
		std::string text = "";
		Logging::LogLevel level = Logging::LogLevel::log;
	};
	std::vector<CachedLog> _messages;
    size_t _listenerIndex;
public:
	ConsoleWindow(GUI& ui, GUIWindowID id);
    ~ConsoleWindow();
	void draw() override;
};


#endif //BRANEENGINE_CONSOLEWINDOW_H
