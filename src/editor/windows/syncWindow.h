//
// Created by eli on 8/15/2022.
//

#ifndef BRANEENGINE_SYNCWINDOW_H
#define BRANEENGINE_SYNCWINDOW_H

#include "networking/connection.h"
#include "ui/guiWindow.h"
#include <atomic>

class SyncWindow : public GUIWindow
{
	//Login variables
	static std::atomic_bool _loggedIn;
	static std::atomic_bool _loggingIn;
	std::string _serverAddress;
	std::string _port;
	std::string _username;
	std::string _password;
	static std::string _feedbackMessage;
	bool _saveUsername;
	void drawSetupConnection();

	//sync variables
	static net::Connection* _syncServer;
	void drawConnected();
public:
	SyncWindow(GUI& ui);
	void displayContent() override;
	static net::Connection* syncServer();
};


#endif //BRANEENGINE_SYNCWINDOW_H
