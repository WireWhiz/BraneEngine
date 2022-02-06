#pragma once
#include "asset.h"
#include <networking/connection.h>

class AssetProvider
{
	void provideAsset(net::Connection* client, Asset* asset);
};

class AssetReceiver
{
	void downloadDependencies(Asset* asset);
};
