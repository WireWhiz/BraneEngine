//
// Created by wirewhiz on 1/19/22.
//

#include <iostream>
#include "gltfLoader.h"

bool gltfLoader::loadGltfFromFile(const std::string& gltfFilename, const std::string& binFilename)
{
    std::ifstream jsonFile(gltfFilename, std::ios::binary);
    if(!jsonFile.is_open())
        return false;
    try{
        jsonFile >> _json;
    }
    catch(const std::exception& e){
        std::cerr << "Json parsing error: " << e.what() << std::endl;
        return false;
    }

    jsonFile.close();
    std::unique_ptr<std::ifstream> binFile = std::make_unique<std::ifstream>(binFilename, std::ios::binary); // File will be closed when this object is destroyed.
    if(!binFile->is_open())
        return false;
    _bin = std::move(binFile);
    _usingFile = true;
    return true;
}

gltfLoader::~gltfLoader()
{

}
#include <unordered_set>
void gltfLoader::printInfo()
{
    std::cout << "meshes: " << _json["meshes"].size() << std::endl;
    size_t primitives = 0;
    size_t verts = 0;
    std::unordered_set<size_t> accessors;
    for (Json::Value& mesh : _json["meshes"])
    {
        primitives += mesh["primitives"].size();
        for(Json::Value& primitive : mesh["primitives"])
        {
            int position = primitive["attributes"]["POSITION"].asInt();
            if(!accessors.count(position))
            {
                accessors.insert(position);
                verts += _json["accessors"][position]["count"].asLargestUInt();
            }
        }
    }
    std::cout << "primitives: " << primitives << std::endl;
    std::cout << "vertices: " << verts << std::endl;

}

void gltfLoader::printPositions(int meshIndex, int primitiveIndex)
{
    Json::Value& primitive = _json["meshes"][meshIndex]["primitives"][primitiveIndex];
    Json::Value& positionAccessor = _json["accessors"][primitive["attributes"]["POSITION"].asInt()];
    Json::Value& bufferView = _json["bufferViews"][positionAccessor["bufferView"].asInt()];

    _bin->seekg(bufferView["byteOffset"].asInt()); // Set stream to read from buffer position
    std::vector<float> buffer(positionAccessor["count"].asInt()  * sizeof(float) * 3);
    _bin->read((char*)buffer.data(), positionAccessor["count"].asInt() * sizeof(float) * 3); // Extract buffer from file

    for (int i = 0; i < positionAccessor["count"].asInt(); ++i)
    {
        std::cout << "(" << buffer[i*3] << ", " << buffer[i*3 + 1] << ", " << buffer[i*3 + 2] << ")" << std::endl;
    }

    std::cout << "vertices: " << positionAccessor["count"].asInt() << std::endl;
}
