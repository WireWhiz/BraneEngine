//
// Created by wirewhiz on 21/07/22.
//

#include "serverFilesystem.h"
#include "utility/strCaseCompare.h"
#include "editorEvents.h"
#include "ui/gui.h"
#include <algorithm>

std::string ServerDirectory::path() const
{
    if(!parent)
        return "";
    else
        return parent->path() + name + "/";
}

bool ServerDirectory::hasParent(ServerDirectory* dir) const
{
    if(!parent)
        return false;
    if(parent == dir)
        return true;
    return parent->hasParent(dir);
}

void ServerDirectory::setParentsOpen()
{
    if(!parent)
        return;
    parent->open = true;
    parent->setParentsOpen();

}

const char *ServerFilesystem::name()
{
    return "serverFilesystem";
}

ServerDirectory* ServerFilesystem::root()
{
    return &_root;
}

void ServerFilesystem::createDirectory(ServerDirectory* parent, const std::string& name)
{
    net::Request req("createDirectory");
    req.body() << (parent->path() + name);
    _server->sendRequest(req).then([this, parent](ISerializedData sData){
        bool success;
        sData >> success;
        if(!success)
        {
            Runtime::error("Could not create directory");
            return;
        }
        fetchDirectory(parent);
    });
}

void ServerFilesystem::setServer(net::Connection* server)
{
    _server = server;
    fetchDirectory(&_root);
}

void ServerFilesystem::deleteFile(ServerDirectory* parent, const std::string& name)
{
    assert(parent);
    net::Request req("deleteFile");
    req.body() << (parent->path() + name);
    _server->sendRequest(req).then([this, parent, name](ISerializedData sData){
        bool success;
        sData >> success;
        if(!success)
        {
            Runtime::error("Could not delete file: " + name);
            return;
        }
        fetchDirectory(parent);
    });
}

void ServerFilesystem::fetchDirectory(ServerDirectory* dir)
{
    net::Request req("directoryContents");
    req.body() << dir->path();

    _server->sendRequest(req).then([this, dir](ISerializedData sData){

        bool success;
        sData >> success;
        if(!success)
        {
            Runtime::error("Could not fetch contents of: " + dir->path());
            return;
        }
        std::scoped_lock l(_directoryLock);
        auto oldChildren = std::move(dir->children);
        dir->files.resize(0);

        std::vector<std::string> directories;
        sData >> directories;
        for(auto& d : directories)
        {
            bool exists = false;
            for (auto& od : oldChildren)
            {
                if(od && od->name == d)
                {
                    exists = true;
                    dir->children.push_back(std::move(od));
                    break;
                }
            }
            if(!exists)
            {
                auto newDir = std::make_unique<ServerDirectory>();
                newDir->name = d;
                newDir->parent = dir;
                fetchDirectory(newDir.get());
                dir->children.push_back(std::move(newDir));
            }
        }
        uint16_t fileCount;
        sData >> fileCount;
        for(size_t i = 0; i < fileCount; i++)
        {
            ServerFile file;
            sData >> file.name >> file.isAsset;
            if(file.isAsset)
                sData >> file.assetID;
            dir->files.push_back(std::move(file));
        }

        dir->loaded = true;
    });
}

void ServerFilesystem::moveDirectory(ServerDirectory* target, ServerDirectory* destination)
{
    assert(target != &_root);
    if(target->parent == destination) //Don't do anything if we're already in the state we want
        return;
    if(destination->hasParent(target)) //We can't move a parent folder into a child
        return;
    net::Request req("moveDirectory");
    req.body() << target->path() << (destination->path() + target->name);

    _server->sendRequest(req).then([this, target, destination](ISerializedData sData) {
        bool success;
        sData >> success;
        if (!success)
        {
            Runtime::error("Could not move file");
            return;
        }
        std::scoped_lock l(_directoryLock);
        auto& children = target->parent->children;
        std::unique_ptr<ServerDirectory> movedDir;
        for(auto i = children.begin(); i != children.end(); i++){
            if(i->get() == target)
            {
                movedDir = std::move(*i);
                children.erase(i);
                break;
            }
        }
        assert(movedDir);
        target->parent = destination;
        destination->children.push_back(std::move(movedDir));
        std::sort(destination->children.begin(), destination->children.end(), [](auto& a, auto& b){
            return strCaseCompare(a->name, b->name);
        });
    });
}

void ServerFilesystem::start()
{
    Runtime::getModule<GUI>()->addEventListener("login", std::function([this](const LoginEvent* login){
        setServer(login->server());
    }));
}

void ServerFilesystem::deleteDirectory(ServerDirectory* directory)
{
    deleteFile(directory->parent, directory->name);
}
