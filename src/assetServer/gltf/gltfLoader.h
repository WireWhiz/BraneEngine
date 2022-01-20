//
// Created by wirewhiz on 1/19/22.
//

#ifndef BRANEENGINE_GLTFLOADER_H
#define BRANEENGINE_GLTFLOADER_H
#include <fstream>
#include <string>
#include <json/json.h>
#include <memory>
#include <byte.h>

class gltfLoader
{
    std::vector<byte> _bin;
    Json::Value _json;
    bool _usingFile;
public:
    ~gltfLoader();
    bool loadGltfFromString(const std::string& gltf, const std::string& bin);
    bool loadGlbFromString(const std::string& glb);
    bool loadGltfFromFile(const std::string& gltfFilename);
    bool loadGlbFromFile(const std::string& glbFilename);
    void printInfo();
    void printPositions(int mesh, int primitive);

};


#endif //BRANEENGINE_GLTFLOADER_H
