//
// Created by wirewhiz on 1/19/22.
//

#ifndef BRANEENGINE_GLTFLOADER_H
#define BRANEENGINE_GLTFLOADER_H
#include <fstream>
#include <string>
#include <json/json.h>
#include <memory>

class gltfLoader
{
    std::unique_ptr<std::istream> _bin;
    Json::Value _json;
    bool _usingFile;
public:
    ~gltfLoader();
    bool loadGltfFromString();
    bool loadGlbFromString();
    bool loadGltfFromFile(const std::string& gltfFilename, const std::string& binFilename);
    bool loadGlbFromFile(const std::string& glbFilename);
    void printInfo();
    void printPositions(int mesh, int primitive);

};


#endif //BRANEENGINE_GLTFLOADER_H
