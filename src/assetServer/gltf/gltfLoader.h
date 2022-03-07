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
#include <assets/types/meshAsset.h>

class gltfLoader
{
    std::vector<byte> _bin;
    Json::Value _json;
    bool _usingFile;

	template<typename T>
	void concatVector(std::vector<T>& dest, const std::vector<T>& src)
	{
		size_t end = dest.size();
		dest.resize(dest.size() + src.size());
		std::memcpy(&dest[end], src.data(), src.size() * sizeof(T));
	}

public:
    ~gltfLoader();
    bool loadGltfFromString(const std::string& gltf, const std::string& bin);
    bool loadGlbFromString(const std::string& glb);
    bool loadGltfFromFile(const std::string& gltfFilename);
    bool loadGlbFromFile(const std::string& glbFilename);
    void printInfo();
    void printPositions(int mesh, int primitive);
	std::vector<uint16_t> readScalarBuffer(uint32_t accessor);
	std::vector<glm::vec2> readVec2Buffer(uint32_t accessor);
	std::vector<glm::vec3> readVec3Buffer(uint32_t accessor);
	std::vector<MeshAsset*> extractAllMeshes();
	Json::Value& nodes();
};


#endif //BRANEENGINE_GLTFLOADER_H
