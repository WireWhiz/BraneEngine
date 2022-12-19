//
// Created by eli on 9/18/2022.
//

#ifndef BRANEENGINE_CHUNKLOADER_H
#define BRANEENGINE_CHUNKLOADER_H

#include "robin_hood.h"
#include "assets/assetID.h"
#include "assets/chunk.h"
#include <shared_mutex>

using ChunkCallbackID = uint32_t;
using ChunkLODCallback = std::function<void(const WorldChunk* chunk, uint32_t oldLOD, uint32_t newLOD)>;
#define NullLOD uint32_t(-1)

class ChunkLoader : public Module
{
    struct ChunkContext
    {
        WorldChunk* chunk;
        uint32_t lod = NullLOD;
        robin_hood::unordered_set<AssetID> _dependencies;
    };
    std::shared_mutex _chunkLock;
    robin_hood::unordered_node_map<AssetID, ChunkContext> _chunks;

    staticIndexVector<ChunkLODCallback> _onLODChange;
    void initAssets(ChunkContext& chunk);
public:
    void loadChunk(WorldChunk* chunk);
    void unloadChunk(const AssetID& chunk);
    uint32_t currentLOD(const AssetID& chunk) const;
    ChunkCallbackID addOnLODChangeCallback(ChunkLODCallback callback);
    void removeOnLODChangeCallback(ChunkCallbackID id);
    void setChunkLOD(const AssetID& chunk, uint32_t lod);

    static const char* name();
};


#endif //BRANEENGINE_CHUNKLOADER_H
