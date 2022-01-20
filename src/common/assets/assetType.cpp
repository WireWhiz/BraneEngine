#include "assetType.h"

AssetType::Type AssetType::fromString(const std::string& type)
{
	static const std::unordered_map<std::string, AssetType::Type> _toEnumMap = {
			{"component", AssetType::component},
			{"system",    AssetType::system},
			{"mesh",      AssetType::mesh},
			{"texture",   AssetType::texture},
			{"player",    AssetType::player},
			{"script",    AssetType::script},
			{"shader",    AssetType::shader},
			{"loginData",    AssetType::loginData}
	};
	assert(_toEnumMap.count(type));
	return _toEnumMap.at(type);
}

std::string AssetType::toString(Type type)
{
	static const std::unordered_map<AssetType::Type, const std::string> _toStringMap = {
			{AssetType::component, "component"},
			{AssetType::system,    "system"},
			{AssetType::mesh,      "mesh"},
			{AssetType::texture,   "texture"},
			{AssetType::player,    "player"},
			{AssetType::script,    "script"},
			{AssetType::shader,    "shader"},
			{AssetType::loginData,    "loginData"}
	};
	assert(_toStringMap.count(type));
	return _toStringMap.at(type);
}

AssetType::AssetType()
{
	_type = none;
}

void AssetType::set(const std::string& type)
{
	
	_type = fromString(type);
}
void AssetType::set(AssetType::Type type)
{
	_type = type;
}

AssetType::Type AssetType::type() const
{
	return _type;
}

std::string AssetType::string() const
{
	return toString(_type);
}

bool AssetType::operator==(AssetType::Type t) const
{
	return t == _type;
}

bool AssetType::operator==(AssetType t) const
{
	return _type == t._type;
}
bool AssetType::operator!=(AssetType t) const
{
	return _type != t._type;
}
