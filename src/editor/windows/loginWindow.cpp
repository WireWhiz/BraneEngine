//
// Created by eli on 5/16/2022.
//

#include "loginWindow.h"
#include "misc/cpp/imgui_stdlib.h"
#include <iostream>
#include "common/config/config.h"
#include "common/networking/networking.h"
#include <ui/gui.h>
#include "../editorEvents.h"

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
		bool enterPressed = ImGui::IsKeyReleased(ImGuiKey_Enter) && ImGui::IsItemFocused();

		ImGui::Separator();

		ImGui::Checkbox("Save username", &_saveUsername);

		bool disableButton = _loggingIn;
		if (disableButton)
			ImGui::BeginDisabled();
		if((ImGui::Button("Submit") || enterPressed) && !_loggingIn && !_loggedIn)
		{
			_loggingIn = true;
            _feedbackMessage = "Connecting...";
			NetworkManager* nm = Runtime::getModule<NetworkManager>();
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
							_loggingIn = false;
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
						_ui.sendEvent(std::make_unique<LoginEvent>(server));
					});

				}
				else
				{
					_loggingIn = false;
					_feedbackMessage = "Unable to connect to server";
				}
			});

		}
		if(disableButton)
			ImGui::EndDisabled();
		ImGui::Text("%s", _feedbackMessage.c_str());
	}
	ImGui::End();
}

LoginWindow::LoginWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	_serverAddress = Config::json()["network"]["asset_server"].asString();
	_port = Config::json()["network"]["tcp_port"].asString();
	_username = Config::json()["user"]["name"].asString();
}
