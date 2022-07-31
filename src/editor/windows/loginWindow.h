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


class LoginWindow : public GUIWindow
{
	std::atomic_bool _loggedIn = false;
	std::atomic_bool _loggingIn = false;
	std::string _serverAddress;
	std::string _port;
	std::string _username;
	std::string _password;
	std::string _feedbackMessage;
	bool _saveUsername;
    void displayContent() override;
public:
    void draw() override;
    LoginWindow(GUI& ui);
};


#endif //BRANEENGINE_LOGINWINDOW_H
