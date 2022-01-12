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
		script,
		player,
		loginData
	};

private:
	Type _type;
	Type fromString(const std::string& type) const;
	std::string toString(Type type) const;


public:
	AssetType();
	
	void set(Type type);
	void set(const std::string& type);
	
	Type type() const;
	std::string string() const;

	bool operator==(Type t) const;
	bool operator==(AssetType t) const;
	bool operator!=(AssetType t) const;
};
