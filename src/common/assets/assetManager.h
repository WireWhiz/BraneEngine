#pragma once
#include "types/assetTypes.h"
#include "asset.h"
#include "assembly.h"
#include <unordered_set>
#include <ecs/ecs.h>
#include <fileManager/fileManager.h>
#include <networking/networking.h>
#include <thread>
#include <utility/asyncQueue.h>

#include <runtime/module.h>
#include <runtime/runtime.h>

class AssetManager : public Module
{
	std::unordered_map<AssetID, std::unique_ptr<Asset>> _assets;
	FileManager& _fm;
	NetworkManager& _nm;
	std::mutex assetLock;


	AsyncQueue<std::pair<Assembly*, EntityID>> _stagedAssemblies;
	std::unordered_map<AssetType::Type, std::vector<std::function<void(Asset* asset)>>> _assetPreprocessors;

	void async_loadAssembly(AssetID assembly, EntityID rootID);
public:
	bool isServer = false;
	AssetManager(Runtime& runtime);



	template<typename T>
	T* getAsset(const AssetID& id)
	{
		static_assert(std::is_base_of<Asset, T>());
		assetLock.lock();
		if(_assets.count(id))
		{
			assetLock.unlock();
			return (T*)(_assets[id].get());
		}
		assetLock.unlock();
		if(isServer && id.serverAddress == "localhost")
		{
			T* asset;
			if constexpr(std::is_same<Asset, T>().value)
				asset = _fm.readUnknownAsset(id, *this);
			else
				asset = _fm.readAsset<T>(id, *this);
			assetLock.lock();
			_assets.insert({id, std::unique_ptr<Asset>(asset)});
			assetLock.unlock();
			return asset;
		}
		else
		{
			if(id.serverAddress.empty())
				throw std::runtime_error("invalid server address");
			return nullptr; //(T*)_nm.requestAsset(id, *this);
		}
	}

	template<typename T>
	AsyncData<T*> async_getAsset(const AssetID& id)
	{
		AsyncData<T*> asset;
		static_assert(std::is_base_of<Asset, T>());
		{
			std::scoped_lock l(assetLock);
			if (_assets.count(id))
			{
				asset.setData((T*)(_assets[id].get()));
				return asset;
			}
		}

        if (isServer && id.serverAddress == "localhost")
        {
            if constexpr(std::is_same<Asset, T>().value)
            {
	            _fm.async_readUnknownAsset(id, *this).then([this, asset, id](T* data){
		            addAsset(data);
		            asset.setData(std::move(data));
	            });
			}
            else
            {
	            _fm.async_readAsset<T>(id, *this).then([this, asset, id](T* data){
		            addAsset(data);
		            asset.setData(std::move(data));
	            });
			}


        }
		else
        {
	        //Create a callback in-between the one that was passed in so that we can save it
			_nm.async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), [this, id, asset](bool connected){
				if(!connected)
				{
					std::cerr << "Could not get asset: " << id << std::endl;
					return;
				}
				if constexpr(std::is_base_of<IncrementalAsset, T>())
				{
					_nm.async_requestAssetIncremental(id, *this).then([this, asset, id](Asset* data){
						addAsset(data);
						asset.setData(std::move((T*)data));

					});
				}
				else
				{
					AsyncData<Asset*> assetToSave;
					_nm.async_requestAsset(id, *this).then([this, asset, id](Asset* data){
						addAsset(data);
						asset.setData(std::move((T*)data));
					});
				}
			});
        }
		return asset;
	}

	void addAsset(Asset* asset);
	void updateAsset(Asset* asset);

	template<typename T>
	inline void addNativeComponent()
	{
		ComponentAsset* asset = T::constructDef();
		_assets.insert({asset->id, std::unique_ptr<Asset>(asset)});
	}

	void startAssetLoaderSystem();
	void addAssetPreprocessor(AssetType::Type type, std::function<void(Asset* asset)> processor);

	const char* name() override;
	void start() override;
};
