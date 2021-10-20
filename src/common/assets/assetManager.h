#pragma once
#include "types/assetTypes.h"
#include <unordered_map>


class AssetManager
{
	class AssetContainer
	{
		std::shared_ptr<size_t> _instances;
		void* _asset;
		void (*_destructor)(const void*);
	public:
		AssetContainer();
		~AssetContainer();
		AssetContainer(const AssetContainer& o);

		template <class T>
		AssetContainer(T* asset)
		{
			_instances = std::make_shared<size_t>(1);

			_asset = static_cast<void*>(asset);
			_destructor = [](const void* a) {
				delete static_cast<const T*>(a);
			};
		}

		void* asset() const;
	};
	std::unordered_map<AssetID, AssetContainer> _assets;
public:
	void addComponent(ComponentAsset* component);
	ComponentAsset* getComponent(const AssetID& id);
	void addMesh(MeshAsset* mesh);
	MeshAsset* getMesh(const AssetID& id);
};
