//
// Created by eli on 5/16/2022.
//

#ifndef BRANEENGINE_LOGINWINDOW_H
#define BRANEENGINE_LOGINWINDOW_H

#include "../editorWindow.h"
#include <string>
#include <atomic>

class LoginWindow : public EditorWindow
{
	std::atomic_bool _loggedIn;
	std::string _serverAddress;
	std::string _port;
	std::string _username;
	std::string _password;
	std::string _feedbackMessage;
	bool _saveUsername;
public:
	LoginWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_LOGINWINDOW_H
