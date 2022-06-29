//
// Created by eli on 5/16/2022.
//

#ifndef BRANEENGINE_LOGINWINDOW_H
#define BRANEENGINE_LOGINWINDOW_H

#include <ui/guiWindow.h>
#include <ui/guiEvent.h>
#include <string>
#include <atomic>

namespace net{
	class Connection;
}

class LoginEvent : public GUIEvent{
	net::Connection* _server;
public:
	LoginEvent(const std::string& name, net::Connection* server);
	inline net::Connection* server() const {return _server;}
};

class LoginWindow : public GUIWindow
{
	std::atomic_bool _loggedIn;
	std::string _serverAddress;
	std::string _port;
	std::string _username;
	std::string _password;
	std::string _feedbackMessage;
	bool _saveUsername;
public:
	LoginWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_LOGINWINDOW_H
