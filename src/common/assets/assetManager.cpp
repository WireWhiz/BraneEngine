#include "assetManager.h"
#include "ecs/ecs.h"
#include "networking/serializedData.h"
#include "networking/message.h"
#include "ecs/nativeTypes/transform.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "ecs/core/component.h"

AssetManager::AssetManager(FileManager& fm, NetworkManager& nm) : _fm(fm), _nm(nm)
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

void AssetManager::startAssetLoaderSystem(EntityManager& em)
{
	NativeForEach forEachAssembly({EntityIDComponent::def(), AssemblyRoot::def()}, &em);
	VirtualSystem vs(AssetID("nativeSystem/1"), [this, forEachAssembly](EntityManager& em){

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

	});
	assert(em.addSystem(std::make_unique<VirtualSystem>(vs)));
}

void AssetManager::async_loadAssembly(AssetID id, EntityID rootID)
{
	AsyncData<Assembly*> assembly;
	assembly.callback([this, rootID](Assembly* assembly, bool successful, const std::string& error){
		if(successful)
		{
			//Load dependencies TODO actually load in all of them & implement incremental loading of some
			auto dependencies = std::make_shared<AsyncDataArray<MeshAsset*>>(assembly->meshes.size());
			dependencies->indexLoaded([dependencies](size_t index, MeshAsset* mesh){
					std::cout << "Loaded: " << mesh->name << std::endl;
			});
			dependencies->fullyLoaded([this, assembly, rootID, dependencies](bool successful, const std::string& error){
				_stagedAssemblies.push_back({assembly, rootID});
			});

			size_t index = 0;
			for(auto& mesh : assembly->meshes)
			{
				async_getAsset(AssetID(mesh), (*dependencies)[index++]);
			}

		}
		else
			std::cerr << error << std::endl;
	});
	async_getAsset(id, assembly);
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



