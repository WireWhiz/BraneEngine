#include "assetType.h"

const std::unordered_map<std::string, AssetType::Type> AssetType::_toEnumMap = {
	{"component", AssetType::component},
	{"mesh",   AssetType::mesh},
	{"texture", AssetType::texture},
	{"player",  AssetType::player},
	{"script",  AssetType::script}
};

const std::unordered_map<AssetType::Type, std::string> AssetType::_toStringMap = {
	{AssetType::component, "component"},
	{AssetType::mesh,   "mesh"},
	{AssetType::texture, "texture"},
	{AssetType::player,  "player"},
	{AssetType::script,  "script"}
};

AssetType::AssetType()
{
	_type = none;
}

void AssetType::set(const std::string& type)
{
	assert(_toEnumMap.count(type));
	_type = _toEnumMap.at(type);
}
void AssetType::set(AssetType::Type type)
{
	assert(_toStringMap.count(type));
	_type = type;
}

AssetType::Type AssetType::type() const
{
	return _type;
}

const std::string& AssetType::string() const
{
	return _toStringMap.at(_type);
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