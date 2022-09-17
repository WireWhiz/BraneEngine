//
// Created by eli on 8/15/2022.
//

#ifndef BRANEENGINE_SYNCWINDOW_H
#define BRANEENGINE_SYNCWINDOW_H

#include "networking/connection.h"
#include "editorWindow.h"
#include <atomic>

class Asset;
class SyncWindow : public EditorWindow
{
	//Login variables
	static std::atomic_bool _loggedIn;
	static std::atomic_bool _loggingIn;
	std::string _serverAddress;
	std::string _port;
	std::string _username;
	std::string _password;
	static std::string _feedbackMessage;
	void drawSetupConnection();

	//sync variables
	static net::Connection* _syncServer;
	void drawConnected();
	void displayContent() override;

	struct AssetDiff
	{
		AssetID id;
	};
	std::vector<AssetDiff> _assetDiffs;
	std::atomic_int _assetDiffSynced = -1;
	void syncAssets();
	void updateAsset(Asset* asset);
public:
	SyncWindow(GUI& ui, Editor& editor);
	static net::Connection* syncServer();
};


#endif //BRANEENGINE_SYNCWINDOW_H
