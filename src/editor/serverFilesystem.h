//
// Created by wirewhiz on 21/07/22.
//

#ifndef BRANEENGINE_SERVERFILESYSTEM_H
#define BRANEENGINE_SERVERFILESYSTEM_H

#include <mutex>

#include "runtime/runtime.h"
#include "assets/assetID.h"
#include "networking/connection.h"
#include "assets/assetType.h"
#include <utility/asyncData.h>

class Asset;

struct ServerFile
{
    std::string name;

    bool isAsset = false;
    AssetID assetID;
};

struct ServerDirectory
{
    bool loaded = false;
    bool open = false;
    std::string name;
    ServerDirectory* parent = nullptr;
    std::vector<ServerFile> files;
    std::vector<std::unique_ptr<ServerDirectory>> children;
    std::string path() const;
    bool hasParent(ServerDirectory* dir) const;
    void setParentsOpen();
};

class ServerFilesystem : public Module
{
    ServerDirectory _root;
    net::Connection* _server = nullptr;
    std::mutex _directoryLock;
public:
    struct SearchResult
    {
        AssetID id;
        std::string name;
        AssetType type;
    };
    void start() override;
    void setServer(net::Connection* server);
    ServerDirectory* root();
    void fetchDirectory(ServerDirectory* dir);
    void createDirectory(ServerDirectory* parent, const std::string& name);
    AsyncData<AssetID> saveAsset(ServerDirectory* destination, Asset* asset);
    void updateAsset(Asset* asset);
    void deleteDirectory(ServerDirectory* directory);
    void deleteFile(ServerDirectory* parent, const std::string& name);
    void moveDirectory(ServerDirectory* target, ServerDirectory* destination);
    AsyncData<std::vector<SearchResult>> searchAssets(int start, int count = 0, const std::string& match = "",  AssetType type = AssetType::none);

    static const char* name();
};


#endif //BRANEENGINE_SERVERFILESYSTEM_H
