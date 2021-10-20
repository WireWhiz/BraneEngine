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
		mesh,
		texture,
		script,
		player

	};

private:
	Type _type;
	static const std::unordered_map<std::string, Type> _toEnumMap;
	static const std::unordered_map<Type, std::string> _toStringMap;

public:
	AssetType();
	
	void set(Type type);
	void set(const std::string& type);
	
	Type type() const;
	const std::string& string() const;

	bool operator==(Type t) const;
	bool operator==(AssetType t) const;
	bool operator!=(AssetType t) const;
};
