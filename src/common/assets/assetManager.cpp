#include "assetManager.h"
#include "ecs/nativeComponent.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "systems/transforms.h"
#include "ecs/entity.h"

AssetManager::AssetManager()
{
}

void AssetManager::addAsset(Asset* asset)
{
    AssetData data{};
    data.asset = std::unique_ptr<Asset>(asset);
    data.loadState = LoadState::loaded;

    _assetLock.lock();
	_assets.insert({asset->id, std::make_unique<AssetData>(std::move(data))});
	_assetLock.unlock();
}

const char* AssetManager::name()
{
	return "assetManager";
}

template<typename T>
void AssetManager::addNativeComponent(EntityManager& em)
{
    static_assert(std::is_base_of<NativeComponent<T>, T>());
    ComponentDescription* description = T::constructDescription();
    AssetID id = AssetID("native", static_cast<uint32_t>(_nativeComponentID++));
    ComponentAsset* asset = new ComponentAsset(T::getMemberTypes(), T::getMemberNames(), id);
    asset->name = T::getComponentName();
    asset->componentID = em.components().registerComponent(description);

    AssetData data{};
    data.asset = std::unique_ptr<Asset>(asset);
    data.loadState = LoadState::loaded;
    _assetLock.lock();
    _assets.insert({asset->id, std::make_unique<AssetData>(std::move(data))});
    _assetLock.unlock();
    description->asset = asset;
}

void AssetManager::start()
{
	EntityManager& em = *Runtime::getModule<EntityManager>();
	addNativeComponent<EntityIDComponent>(em);
    addNativeComponent<EntityName>(em);
	addNativeComponent<Transform>(em);
	addNativeComponent<LocalTransform>(em);
	addNativeComponent<Children>(em);
	addNativeComponent<TRS>(em);
	addNativeComponent<MeshRendererComponent>(em);
}

bool AssetManager::hasAsset(const AssetID& id)
{
	return _assets.count(id);
}

void AssetManager::fetchDependencies(Asset* a, const std::function<void()>& callback)
{
	assert(a);
	assert(a->id != AssetID::null);
	auto deps = a->dependencies();
    if(dependenciesLoaded(a))
    {
		_assetLock.lock();
		for(auto& d : deps)
		{
			if(!_assets[d.id]->usedBy.count(a->id))
				_assets[d.id]->usedBy.insert(a->id);
		}
		_assetLock.unlock();
        callback();
        return;
    }

    auto unloaded = std::make_shared<size_t>(deps.size());
    auto callbackPtr = std::make_shared<std::function<void()>>(callback);

    for(auto& d : deps)
    {
        fetchAsset(d.id, d.streamable).then([this, a, unloaded, callbackPtr](Asset* asset)
        {
            Runtime::log("Loaded: " + asset->name);
	        _assetLock.lock();
			assert(_assets.count(asset->id));
			_assets.at(asset->id)->usedBy.insert(a->id);
	        _assetLock.unlock();
            if(--(*unloaded) == 0)
                (*callbackPtr)();
        }).onError([unloaded, d, callbackPtr](const std::string& message){
            Runtime::error("Unable to fetch: " + d.id.string());
            if(--(*unloaded) == 0)
                (*callbackPtr)();
        });
    }
}

bool AssetManager::dependenciesLoaded(const Asset* asset) const
{
    auto deps = asset->dependencies();
    for(auto& d : deps)
        if(!_assets.count(d.id))
            return false;
    return true;
}



