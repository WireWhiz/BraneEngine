#include "assetTypes.h"

const std::array<std::string, 3> AssetType::_types = {
	"model",
	"texture",
	"player"
};

AssetType::AssetType()
{
	_index = 0;
}

void AssetType::set(const std::string& type)
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		if(type == _types[i])
			_index = i;
	}
}

const std::string& AssetType::string()
{
	return _types[_index];
}