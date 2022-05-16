//
// Created by eli on 5/16/2022.
//

#include "LoginWindow.h"
#include <misc/cpp/imgui_stdlib.h>
#include <iostream>
#include <config/config.h>

void LoginWindow::draw()
{
	if(ImGui::Begin("login")){
		auto inputFlags = ImGuiInputTextFlags_AlwaysInsertMode;
		ImGui::InputText("Server", &_serverAddress, inputFlags);
		ImGui::InputText("port", &_port, inputFlags);
		ImGui::InputText("Username", &_username, inputFlags);
		ImGui::InputText("Password", &_password, inputFlags | ImGuiInputTextFlags_Password);
		ImGui::Separator();

		ImGui::Checkbox("Save username", &_saveUsername);

		if(ImGui::Button("Submit"))
		{
			if(_saveUsername)
			{
				Config::json()["user"]["name"] = _username;
				Config::save();
			}
		}

		ImGui::End();
	}
}

LoginWindow::LoginWindow(EditorUI& ui) : EditorWindow(ui)
{
	_serverAddress = Config::json()["network"]["asset_server"].asString();
	_port = Config::json()["network"]["tcp_port"].asString();
	_username = Config::json()["user"]["name"].asString();
}
