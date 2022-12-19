#pragma once
#include <string_view>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <cassert>

class AssetType
{
public:
    enum Type
    {
        none = 0,
        script = 1,
        mesh = 2,
        image = 3,
        shader = 4,
        material = 5,
        assembly = 6,
        chunk = 7,
        player = 8
    };

private:
    Type _type;



public:
    AssetType();
    AssetType(Type type);
    static Type fromString(const std::string& type);
    static const std::string& toString(Type type);
    
    void set(Type type);
    void set(const std::string& type);
    
    [[nodiscard]] Type type() const;
    [[nodiscard]] const std::string& toString() const;

    operator Type() const;
    bool operator==(Type t) const;
    bool operator!=(Type t) const;
    bool operator==(AssetType t) const;
    bool operator!=(AssetType t) const;
};
