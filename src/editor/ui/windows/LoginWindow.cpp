//
// Created by eli on 5/16/2022.
//

#include "LoginWindow.h"
#include <misc/cpp/imgui_stdlib.h>
#include <iostream>
#include <config/config.h>
#include <networking/networking.h>
#include "../editorUI.h"

void LoginWindow::draw()
{
	ImVec2 center = ImGui::GetCurrentContext()->CurrentViewport->GetCenter();
	ImVec2 size(400, 220);
	ImGui::SetNextWindowPos({center.x - size.x / 2, center.y - size.y / 2});
	ImGui::SetNextWindowSize(size);
	if(ImGui::Begin("login", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)){
		ImGui::Text("Select Server:");
		ImGui::InputText("address", &_serverAddress);
		ImGui::InputText("port", &_port);
		ImGui::Separator();
		ImGui::Text("Login:");
		ImGui::InputText("username", &_username);
		ImGui::InputText("password", &_password, ImGuiInputTextFlags_Password);
		ImGui::Separator();

		ImGui::Checkbox("Save username", &_saveUsername);

		if(ImGui::Button("Submit"))
		{
			NetworkManager* nm = (NetworkManager*)_ui.runtime().getModule("networkManager");
			nm->async_connectToAssetServer(_serverAddress, std::stoi(_port), [this, nm](bool success){
				if(success)
				{
					_feedbackMessage = "Connected, logging in...";
					net::Request req("login");

					req.body() << _username << _password;

					auto* server = nm->getServer(_serverAddress);
					server->sendRequest(req).then([this, server](ISerializedData sData){
						bool result;
						sData >> result;
						if(!result)
						{
							_feedbackMessage = "invalid username/password";
							return;
						}

						_feedbackMessage = "Logged in!";
						_loggedIn = true;
						if(_saveUsername)
						{
							Config::json()["user"]["name"] = _username;
							Config::save();
						}
						_ui.sendEvent(std::make_unique<LoginEvent>("loginSuccessful", server));
					});

				}
				else
				{
					_feedbackMessage = "Unable to connect to server";
				}
			});

		}
		ImGui::Text("%s", _feedbackMessage.c_str());
	}
	ImGui::End();
}

LoginWindow::LoginWindow(EditorUI& ui) : EditorWindow(ui)
{
	_serverAddress = Config::json()["network"]["asset_server"].asString();
	_port = Config::json()["network"]["tcp_port"].asString();
	_username = Config::json()["user"]["name"].asString();
}

LoginEvent::LoginEvent(const std::string& name, net::Connection* server) : EditorEvent(name)
{
	_server = server;
}
