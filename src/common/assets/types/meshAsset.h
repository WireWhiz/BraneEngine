#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../asset.h"

class MeshAsset : public IncrementalAsset
{
	const uint32_t _trisPerIncrement = 1;

    struct Primitive{
        uint32_t indexOffset;
        uint32_t indexCount;
        uint32_t vertexCount;

        struct Attribute{
            uint32_t offset;
            uint32_t step;
        };
        std::unordered_map<std::string, Attribute> attributes;
    };

    std::vector<Primitive> _primitives;
    std::vector<byte> _data;
public:
    struct MeshSerializationContext : SerializationContext
    {
        size_t primitive;
        size_t pos;
        std::vector<bool> vertexSent;
    };

	size_t pipelineID;
	bool meshUpdated;

	MeshAsset();
    size_t addPrimitive(const std::vector<uint16_t>& indices, uint32_t vertexCount);
    template<typename T>
    void addAttribute(size_t primitive, const std::string& name, std::vector<T>& data)
    {
        assert(primitive < _primitives.size());
        size_t index = _data.size();
        size_t newSize = _data.size() + data.size() * sizeof(T);
        newSize += 4 - newSize % 4;
        _data.resize(newSize);
        std::memcpy(&_data[index], data.data(), data.size() * sizeof(T));
        assert(_data.size() % 4 == 0);

        _primitives[primitive].attributes.insert({name, {(uint32_t)index, sizeof(T)}});
    }

    const std::vector<byte>& packedData() const;
    uint32_t indexOffset(size_t primitive) const;
    uint32_t indexCount(size_t primitive) const;
    bool hasAttribute(size_t primitive, const std::string& name) const;
    uint32_t attributeOffset(size_t primitive, const std::string& name) const;
    uint32_t vertexCount(uint32_t primitive) const;


	void toFile(MarkedSerializedData& sData) override;
	void fromFile(MarkedSerializedData& sData, AssetManager& am) override;
	void serialize(OSerializedData& message) override;
	void deserialize(ISerializedData& message, AssetManager& am) override;
	void serializeHeader(OSerializedData& sData) override;
	void deserializeHeader(ISerializedData& sData, AssetManager& am) override;
    std::unique_ptr<SerializationContext> createContext() const override;
	bool serializeIncrement(OSerializedData& sData, SerializationContext* iteratorData) override;
	void deserializeIncrement(ISerializedData& sData) override;


	size_t meshSize() const;
    size_t primitiveCount() const;
};