//
// Created by eli on 9/18/2022.
//

#include "chunkLoader.h"
#include "utility/threadPool.h"
#include "assets/assetManager.h"
#include "graphics/graphics.h"

void ChunkLoader::loadChunk(WorldChunk* chunk)
{

    _chunkLock.lock();
    _chunks.insert({chunk->id, ChunkContext{chunk}});
    _chunkLock.unlock();

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
    _chunkLock.lock_shared();
    assert(_chunks.contains(chunk));
    auto& cctx = _chunks.at(chunk);
    _chunkLock.unlock_shared();
    assert(lod <= cctx.chunk->maxLOD);

    uint32_t oldLod = cctx.lod;
    cctx.lod = lod;

    std::vector<WorldChunk::LOD*> LODsToLoad;
    for(auto& l : cctx.chunk->LODs)
    {
        if(l.min <= lod && lod <= l.max)
            LODsToLoad.push_back(&l);
    }

    auto cJob = ThreadPool::conditionalEnqueue([this, cctx, oldLod, lod]()mutable {
        initAssets(cctx);
        for(auto& callback : _onLODChange)
            callback(cctx.chunk, oldLod, lod);
    }, LODsToLoad.size());

    auto* am = Runtime::getModule<AssetManager>();
    for(auto& l : LODsToLoad)
    {
        AssetID id = l->assembly;
        if(id.address().empty())
            id.setAddress(cctx.chunk->id.address());
        am->fetchAsset<Assembly>(id).then([cJob](Assembly* asset){
            cJob->signal();
        }).onError([](const std::string& error){
            Runtime::error("Could not load chunk lod: " + error);
        });
    }
}

const char* ChunkLoader::name()
{
    return "chunkLoader";
}

void ChunkLoader::initAssets(ChunkLoader::ChunkContext& chunk)
{
    auto am = Runtime::getModule<AssetManager>();
    robin_hood::unordered_set<AssetID> deps;
    for(auto& l : chunk.chunk->LODs)
    {
        if(l.min > chunk.lod || chunk.lod > l.max)
            continue;
        am->getDependenciesRecursive(l.assembly, deps);
    }

    auto vkr = Runtime::getModule<graphics::VulkanRuntime>();
    for(auto& dep : deps)
    {
        auto* asset = am->getAsset<Asset>(dep);
        if(asset->runtimeID != -1)
            continue;
        switch(asset->type.type())
        {
            case AssetType::script:
                break;
            case AssetType::mesh:
            case AssetType::image:
            case AssetType::shader:
            case AssetType::material:
                if(vkr)
                    asset->runtimeID = vkr->addAsset(asset);
                break;
            case AssetType::assembly:
                break;
            default:
                Runtime::error("Attempted to initialize unknown asset type: " + asset->type.toString());
                assert(false);
        }
    }
}

uint32_t ChunkLoader::currentLOD(const AssetID& chunk) const
{
    if(!_chunks.contains(chunk))
        return 0;
    return _chunks.at(chunk).lod;
}
