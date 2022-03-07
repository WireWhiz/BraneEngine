#pragma once
#include "types/assetTypes.h"
#include "asset.h"
#include <unordered_set>
#include <ecs/ecs.h>
#include <fileManager/fileManager.h>
#include <networking/networking.h>

class AssetManager
{
	std::unordered_map<AssetID, std::unique_ptr<Asset>> _assets;
	FileManager& _fm;
	NetworkManager& _nm;

public:
	bool isServer = false;
	AssetManager(FileManager& fm, NetworkManager& nm);

	template<typename T>
	T* getAsset(AssetID id)
	{
		if(_assets.count(id))
			return (T*)(_assets[id].get());
		if(isServer && id.serverAddress == "localhost")
		{
			T* asset;
			if constexpr(std::is_same<Asset, T>().value)
				asset = _fm.readUnknownAsset(id, *this);
			else
				asset = _fm.readAsset<T>(id, *this);
			_assets.insert({id, std::unique_ptr<Asset>(asset)});
			return asset;
		}
		else
		{
			return (T*)_nm.requestAsset(id, *this);;
		}
	}

	std::string getAssetName(AssetID& id);

	template<typename T>
	inline void addNativeComponent()
	{
		ComponentAsset* asset = T::constructDef();
		_assets.insert({asset->id, std::unique_ptr<Asset>(asset)});
	}

};
