#include "assetManager.h"
#include "ecs/ecs.h"
#include <utility/serializedData.h>
#include "networking/message.h"
#include "ecs/nativeTypes/transform.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "ecs/core/component.h"

AssetManager::AssetManager(Runtime& runtime) :
	_fm(*((FileManager*)runtime.getModule("fileManager"))),
	_nm(*((NetworkManager*)runtime.getModule("networkManager"))),
	Module(runtime)
{
	addNativeComponent<EntityIDComponent>();
	addNativeComponent<TransformComponent>();
	addNativeComponent<LocalTransformComponent>();
	addNativeComponent<ChildrenComponent>();
	addNativeComponent<MeshRendererComponent>();
	addNativeComponent<AssemblyRoot>();
}

void AssetManager::updateAsset(Asset* asset)
{
	std::scoped_lock l(assetLock);
	if(!_assets.count(asset->id))
	{
		_assets.insert({asset->id, std::unique_ptr<Asset>(asset)});
		return;
	}

	*_assets[asset->id] = std::move(*asset);

}

void AssetManager::startAssetLoaderSystem()
{
	EntityManager& em = *(EntityManager*)_rt.getModule("entityManager");
	NativeForEach forEachAssembly({EntityIDComponent::def(), AssemblyRoot::def()}, &em);
	_rt.timeline().addTask("asset loader", [this, forEachAssembly, &em](){

		//Start loading all unloaded assemblies TODO change it so that the state is stored through components and not a bool, so we're not always iterating over literally everything
		em.forEach(forEachAssembly.id(), [&](byte** components){
			AssemblyRoot* ar = AssemblyRoot::fromVirtual(components[forEachAssembly.getComponentIndex(1)]);
			if(!ar->loaded)
			{
				ar->loaded = true;
				EntityIDComponent* id = EntityIDComponent::fromVirtual(components[forEachAssembly.getComponentIndex(0)]);
				async_loadAssembly(ar->id, id->id);
				std::cout << "loading assembly: " << id-id << std::endl;
			}
		});

		// Finish loading assets that we have gotten back from the asset manager or file manager
		for(auto a : _stagedAssemblies)
		{
			//Run preprocessors now, instead of right when we got the asset data
			std::cout << "Pre-processing: " << a.first->name << std::endl;
			for(auto& f : _assetPreprocessors[AssetType::assembly])
				f(a.first);
			std::cout << "Injecting: " << a.first->name << std::endl;
			a.first->inject(em, a.second);
		}
		_stagedAssemblies.clear();

	}, "asset management");
}

void AssetManager::async_loadAssembly(AssetID id, EntityID rootID)
{
	async_getAsset<Assembly>(id).then([this, rootID](Assembly* assembly){
		//Load dependencies TODO actually load in all of them & implement incremental loading of some
		std::cout << "Loaded: " << assembly->name << " requesting dependencies" << std::endl;
		std::shared_ptr<size_t> unloaded = std::make_unique<size_t>();
		*unloaded = assembly->meshes.size();
		for(auto& mesh : assembly->meshes)
		{
			async_getAsset<MeshAsset>(AssetID(mesh)).then([this, unloaded, assembly, rootID](MeshAsset* mesh)
			{
				std::cout << "Loaded: " << mesh->name << std::endl;
				if(--(*unloaded) == 0)
					_stagedAssemblies.push_back({assembly, rootID});
			});
		}
	});
}

void AssetManager::addAssetPreprocessor(AssetType::Type type, std::function<void(Asset*)> processor)
{
	_assetPreprocessors[type].push_back(processor);
}

void AssetManager::addAsset(Asset* asset)
{
	if(asset->type != AssetType::assembly)
		for(auto& f : _assetPreprocessors[asset->type.type()])
			f(asset);

	assetLock.lock();
	_assets.insert({asset->id, std::unique_ptr<Asset>(asset)});
	assetLock.unlock();
}

const char* AssetManager::name()
{
	return "assetManager";
}

void AssetManager::start()
{
	startAssetLoaderSystem();
}



