//
// Created by eli on 8/15/2022.
//

#include "syncWindow.h"
#include "editor/editorEvents.h"
#include "networking/networking.h"

std::atomic_bool SyncWindow::_loggedIn = false;
std::atomic_bool SyncWindow::_loggingIn = false;
net::Connection* SyncWindow::_syncServer = nullptr;
std::string SyncWindow::_feedbackMessage;

SyncWindow::SyncWindow(GUI& ui) : GUIWindow(ui)
{
	_name = "Sync";
}

void SyncWindow::displayContent()
{
	if(_loggedIn)
		drawConnected();
	else
		drawSetupConnection();
}

void SyncWindow::drawSetupConnection()
{
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
				SerializedData req;
				OutputSerializer s(req);
				s << _username << _password;

				auto* server = nm->getServer(_serverAddress);
				server->sendRequest("login", std::move(req), [this, server](net::ResponseCode code, InputSerializer sData){
					if(code != net::ResponseCode::success)
					{
						_loggingIn = false;
						if(code == net::ResponseCode::denied)
							_feedbackMessage = "invalid username/password";
						else
							_feedbackMessage = "request error: " + std::to_string((uint8_t)code);
						return;
					}

					_feedbackMessage = "Logged in!";
					_loggedIn = true;
					_loggingIn = false;
					if(_saveUsername)
					{
						Config::json()["user"]["name"] = _username;
						Config::save();
					}
					_syncServer = server;
					_syncServer->onDisconnect([](){
						//Don't need a "this" pointer as they're static
						_loggedIn = false;
						_syncServer = nullptr;
						_feedbackMessage = "Disconnected from server!";
					});
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

net::Connection* SyncWindow::syncServer()
{
	return _syncServer;
}

void SyncWindow::drawConnected()
{
	ImGui::Text("TODO: create a way to sync local assets that have changed to the asset server");

}
