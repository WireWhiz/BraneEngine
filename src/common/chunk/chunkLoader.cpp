//
// Created by eli on 9/18/2022.
//

#include "chunkLoader.h"
#include "utility/threadPool.h"
#include "assets/assetManager.h"

void ChunkLoader::loadChunk(WorldChunk* chunk)
{
	std::scoped_lock l(_chunkLock);
	_chunks.insert({chunk->id, {chunk}});

	assert(chunk->maxLOD != NullLOD);
	if(chunk->maxLOD != NullLOD)
		setChunkLOD(chunk->id, chunk->maxLOD);
}

void ChunkLoader::unloadChunk(const AssetID& chunk)
{
	setChunkLOD(chunk, NullLOD);
	std::scoped_lock l(_chunkLock);
	_chunks.erase(chunk);
}

ChunkCallbackID ChunkLoader::addOnLODChangeCallback(ChunkLODCallback callback)
{
	return _onLODChange.push(std::move(callback));
}

void ChunkLoader::removeOnLODChangeCallback(ChunkCallbackID id)
{
	assert(_onLODChange.hasIndex(id));
	_onLODChange.remove(id);
}

void ChunkLoader::setChunkLOD(const AssetID& chunk, uint32_t lod)
{
	std::scoped_lock lock(_chunkLock);
	assert(_chunks.contains(chunk));
	auto& cctx = _chunks.at(chunk);
	assert(lod <= cctx.chunk->maxLOD);

	std::vector<WorldChunk::LOD*> LODsToLoad;
	for(auto& l : cctx.chunk->LODs)
	{
		if(l.minLOD <= lod && lod <= l.maxLOD)
			LODsToLoad.push_back(&l);
	}

	WorldChunk* chunkPtr = cctx.chunk;

	auto cJob = ThreadPool::conditionalEnqueue([this, chunkPtr, lod](){
		for(auto& callback : _onLODChange)
			callback(chunkPtr, lod);
	}, LODsToLoad.size());

	auto* am =Runtime::getModule<AssetManager>();
	for(auto& l : LODsToLoad)
	{
		am->fetchAsset<Assembly>(l->assembly).then([cJob](Assembly* asset){
			cJob->signal();
		}).onError([](const std::string& error){
			Runtime::error("Could not load chunk lod: " + error);
		});
	}
}
