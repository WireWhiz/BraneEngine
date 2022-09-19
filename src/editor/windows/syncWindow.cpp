//
// Created by eli on 8/15/2022.
//

#include "syncWindow.h"
#include "editor/editorEvents.h"
#include "networking/networking.h"
#include "editor/editor.h"
#include "utility/threadPool.h"
#include "ui/IconsFontAwesome6.h"
#include "assets/asset.h"
#include "assets/assetManager.h"

std::atomic_bool SyncWindow::_loggedIn = false;
std::atomic_bool SyncWindow::_loggingIn = false;
net::Connection* SyncWindow::_syncServer = nullptr;
std::string SyncWindow::_feedbackMessage;

SyncWindow::SyncWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor)
{
	_name = "Sync";

	auto& project = _editor.project().json().data();
	_serverAddress = project["server"].get("address", "localhost").asString();
	_port = project["server"].get("port", "2001").asString();
	_username = project["server"].get("username", "").asString();
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

					auto& project = _editor.project().json().data();
					project["server"]["address"] = _serverAddress;
					project["server"]["port"] = _port;
					project["server"]["username"] = _username;

					_syncServer = server;
					_syncServer->onDisconnect([](){
						//Don't need a "this" pointer as they're static
						_loggedIn = false;
						_loggingIn = false;
						_syncServer = nullptr;
						_feedbackMessage = "Disconnected from server!";
					});
				});

			}
			else
			{
				_loggedIn = false;
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
	if(ImGui::BeginTabBar("SubWindows"))
	{
		if(ImGui::BeginTabItem("Sync Assets"))
		{
			syncAssets();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Server Settings"))
		{
			ImGui::TextDisabled("Work in progress");
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Users"))
		{
			ImGui::TextDisabled("Work in progress");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

}

void SyncWindow::syncAssets()
{
	if(_assetDiffSynced == -1)
	{
		ThreadPool::enqueue([this]{
			SerializedData assetHashes;
			OutputSerializer s(assetHashes);
			std::vector<std::pair<AssetID, std::string>> hashes = _editor.project().getAssetHashes();

			uint32_t diffs = hashes.size();
			s << diffs;
			for(auto& h : hashes)
				s << h.first << h.second;

			_syncServer->sendRequest("getAssetDiff", std::move(assetHashes), [this](auto code, InputSerializer res){
				uint32_t diffs;
				res >> diffs;
				_assetDiffs.resize(diffs);
				for (uint32_t i = 0; i < diffs; ++i)
					res >> _assetDiffs[i].id;
				_assetDiffSynced = 1;
			});
		});
		_assetDiffSynced = 0;
	}
	if(_assetDiffSynced == 0)
	{
		const char* dots[] = {"", ".", "..", "..."};
		size_t time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 4;
		ImGui::Text("Comparing assets%s", dots[time]);
		return;
	}

	ImGui::Text("Changed Assets: ");
	ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 68);
	if(ImGui::Button("Sync All " ICON_FA_CLOUD_ARROW_UP))
	{
		for(auto& asset : _assetDiffs)
		{
			Runtime::getModule<AssetManager>()->fetchAsset(asset.id).then([this](Asset* asset){
				updateAsset(asset);
			});
		}
		_assetDiffSynced = -1;
	}
	for(auto& asset : _assetDiffs)
	{
		ImGui::PushID(asset.id.id());
		ImGui::Selectable(_editor.project().getAssetName(asset.id).c_str());
		if(ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", asset.id.string().c_str());
		ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 20);
		if(ImGui::Button(ICON_FA_CLOUD_ARROW_UP))
		{
			Runtime::getModule<AssetManager>()->fetchAsset(asset.id).then([this](Asset* asset){
				updateAsset(asset);
			});
			_assetDiffSynced = -1;
		}
		if(ImGui::IsItemHovered())
			ImGui::SetTooltip("Sync Asset");
		ImGui::PopID();
	}
}

void SyncWindow::updateAsset(Asset* asset)
{
	SerializedData assetData;
	OutputSerializer s(assetData);
	asset->serialize(s);
	AssetID* id = &asset->id;
	_syncServer->sendRequest("updateAsset", std::move(assetData), [id](auto ec, InputSerializer res){
		if(ec == net::ResponseCode::success)
			Runtime::log("Asset " + id->string() + " updated");
		else
			Runtime::error("Failed to update asset " + id->string());
	});
}
