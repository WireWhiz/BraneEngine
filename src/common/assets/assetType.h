#pragma once
#include <string_view>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <assert.h>

class AssetType
{
public:
	enum Type
	{
		none,
		component,
		system,
		mesh,
		texture,
		shader,
		material,
		assembly,
		player
	};

private:
	Type _type;



public:
	AssetType();
	static Type fromString(const std::string& type);
	static const std::string& toString(Type type);
	
	void set(Type type);
	void set(const std::string& type);
	
	[[nodiscard]] Type type() const;
	[[nodiscard]] const std::string& string() const;

	bool operator==(Type t) const;
	bool operator==(AssetType t) const;
	bool operator!=(AssetType t) const;
};
