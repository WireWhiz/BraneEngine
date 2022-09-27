#include "client.h"
#include "runtime/runtime.h"

#include "networking/networking.h"
#include "graphics/graphics.h"
#include "assets/assembly.h"
#include "systems/transforms.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "assets/assetManager.h"

#include <utility/threadPool.h>
#include <asio.hpp>
#include "networking/connection.h"
#include "chunk/chunkLoader.h"
#include "assets/chunk.h"

const char* Client::name()
{
	return "client";
}

Client::Client()
{


}

void Client::start()
{
	auto* nm = Runtime::getModule<NetworkManager>();
	auto* am = Runtime::getModule<AssetManager>();
	auto* cl = Runtime::getModule<ChunkLoader>();
	auto* em = Runtime::getModule<EntityManager>();

	cl->addOnLODChangeCallback([this, em, am](const WorldChunk* chunk, uint32_t oldLod, uint32_t newLod)
	{
		if(oldLod != NullLOD)
		{
			for(auto& lod : chunk->LODs)
			{
				if(lod.min > newLod || lod.max < newLod)
				{
					assert(_chunkRoots.contains(lod.assembly));
					Runtime::getModule<Transforms>()->destroyRecursive(_chunkRoots.at(lod.assembly));
					_chunkRoots.erase(lod.assembly);
				}
			}
		}
		if(newLod != NullLOD)
		{
			for(auto& lod : chunk->LODs)
			{
				if(lod.min <= newLod && newLod <= lod.max)
				{
					am->fetchAsset<Assembly>(lod.assembly).then([this, em](Assembly* assembly){
						assembly->inject(em);
						_chunkRoots.insert(lod.assembly);
					});
				}
			}
		}
	});

	nm->addRequestListener("loadChunk", [am](auto& rc){
		AssetID chunkID;
		rc.req >> chunkID;
		am->fetchAsset<WorldChunk>(chunkID).then([](WorldChunk* chunk){
			Runtime::getModule<ChunkLoader>()->loadChunk(chunk);
			Runtime::log("Loaded chunk " + chunk->id.string());
		});
		Runtime::log("Was requested to load chunk " + chunkID.string());
	});
	nm->addRequestListener("unloadChunk", [](auto& rc){
		Runtime::log("Was requested to unload chunk");
	});


}

AsyncData<Asset*> AssetManager::fetchAssetInternal(const AssetID& id, bool incremental)
{
	AsyncData<Asset*> asset;
	if(id.address().empty())
	{
		asset.setError("Asset with id " + std::string(id.idStr()) + " was not found and can not be remotely fetched since it lacks a server address");
		return asset;
	}
	auto* nm = Runtime::getModule<NetworkManager>();
	if(incremental)
	{
		nm->async_requestAssetIncremental(id).then([this, asset](Asset* ptr){
			asset.setData(ptr);
		});
	}
	else
	{
		nm->async_requestAsset(id).then([this, asset](Asset* ptr){
			_assetLock.lock();
			_assets.at(ptr->id)->loadState = LoadState::awaitingDependencies;
			_assetLock.unlock();
			if(dependenciesLoaded(ptr))
			{
				asset.setData(ptr);
				return;
			}
			fetchDependencies(ptr, [ptr, asset]() mutable{
				asset.setData(ptr);
			});
		});
	}
	return asset;
}
