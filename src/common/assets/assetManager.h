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

class AssetManager
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
	AssetManager(FileManager& fm, NetworkManager& nm);



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
	void async_getAsset(const AssetID& id, AsyncData<T*> asset)
	{
		static_assert(std::is_base_of<Asset, T>());
		{
			std::scoped_lock l(assetLock);
			if (_assets.count(id))
			{
				asset.setData((T*)(_assets[id].get()));
				return;
			}
		}

        if (isServer && id.serverAddress == "localhost")
        {
	        //Create a callback in-between the one that was passed in so that we can save it
	        AsyncData<T*> assetToSave;
	        assetToSave.callback([this, asset, id](T* data, bool successful, const std::string& error){
		        if(successful)
		        {
					addAsset(data);
			        asset.setData(data);
		        }
		        else
			        asset.setError(error);

	        });
            if constexpr(std::is_same<Asset, T>().value)
                _fm.async_readUnknownAsset(id, *this, assetToSave);
            else
                _fm.async_readAsset<T>(id, *this, assetToSave);

        }
		else
        {
	        //Create a callback in-between the one that was passed in so that we can save it

			if constexpr(std::is_base_of<IncrementalAsset, T>())
			{
				AsyncData<IncrementalAsset*> assetToSave;
				assetToSave.callback([this, asset, id](Asset* data, bool successful, const std::string& error){
					if(successful)
					{
						addAsset(data);
						asset.setData((T*)data);
					}
					else
						asset.setError(error);

				});
				_nm.async_requestAssetIncremental(id, *this, assetToSave);
			}
			else
			{
				AsyncData<Asset*> assetToSave;
				assetToSave.callback([this, asset, id](Asset* data, bool successful, const std::string& error){
					if(successful)
					{
						addAsset(data);
						asset.setData((T*)data);
					}
					else
						asset.setError(error);

				});
				_nm.async_requestAsset(id, *this, assetToSave);
			}

        }
	}

	void addAsset(Asset* asset);
	void updateAsset(Asset* asset);

	template<typename T>
	inline void addNativeComponent()
	{
		ComponentAsset* asset = T::constructDef();
		_assets.insert({asset->id, std::unique_ptr<Asset>(asset)});
	}

	void startAssetLoaderSystem(EntityManager& em);
	void addAssetPreprocessor(AssetType::Type type, std::function<void(Asset* asset)> processor);
};
