#pragma once
#include <stdio.h>
#include <assets/assetManager.h>

class FileManager
{

public:
	Asset* readAsset(AssetID id);
	void writeAsset(Asset* asset);
};