#pragma once
#include "asset.h"
#include "assembly.h"
#include <unordered_set>
#include <fileManager/fileManager.h>
#include <networking/networking.h>
#include <thread>
#include <utility/asyncQueue.h>

#include <runtime/module.h>
#include <runtime/runtime.h>

class AssetManager : public Module
{
public:
	using FetchCallback = std::function<AsyncData<Asset*>(const AssetID& id, bool incremental)>;

private:
	std::mutex assetLock;
	std::unordered_map<AssetID, std::unique_ptr<Asset>> _assets;

	AsyncQueue<std::pair<Assembly*, EntityID>> _stagedAssemblies;
	std::unordered_map<AssetType::Type, std::vector<std::function<void(Asset* asset)>>> _assetPreprocessors;

	FetchCallback _fetchCallback;

	void loadAssembly(AssetID assembly, EntityID rootID);

    size_t _nativeComponentID = 0;
    template<typename T>
    void addNativeComponent(EntityManager& em);
public:
	AssetManager();

	template<typename T>
	T* getAsset(const AssetID& id)
	{
		static_assert(std::is_base_of<Asset, T>());
		std::scoped_lock lock(assetLock);
		if(_assets.count(id))
			return (T*)(_assets[id].get());
		return nullptr;
	}
	void setFetchCallback(std::function<AsyncData<Asset*>(const AssetID& id, bool incremental)> callback);
	AsyncData<Asset*> fetchAsset(const AssetID& id, bool incremental = false);
	template <typename T>
	AsyncData<T*> fetchAsset(const AssetID& id)
	{
		static_assert(std::is_base_of<Asset, T>());
		AsyncData<T*> asset;
		fetchAsset(id, std::is_base_of<IncrementalAsset, T>()).then([asset](Asset* a)
		{
			asset.setData(std::move((T*)a));
		});
		return asset;
	}
	void addAsset(Asset* asset);
	void updateAsset(Asset* asset);
	bool hasAsset(const AssetID& id);

	void startAssetLoaderSystem();
	void addAssetPreprocessor(AssetType::Type type, std::function<void(Asset* asset)> processor);
	void fetchDependencies(Asset* asset, std::function<void()> callback);

	static const char* name();
	void start() override;
};
