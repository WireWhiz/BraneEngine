//
// Created by wirewhiz on 07/08/22.
//

#ifndef BRANEENGINE_ASSETSEARCHWIDGET_H
#define BRANEENGINE_ASSETSEARCHWIDGET_H

#include <string>
#include <mutex>
#include "assets/assetType.h"
#include "assets/assetID.h"
#include <filesystem>
#include <vector>
#include <atomic>

class BraneProject;
class AssetSearchWidget
{
    std::string _searchText;
    size_t _searchIncrement;
    AssetType _assetType;

	BraneProject* _project;
    std::vector<std::pair<AssetID, std::filesystem::path>> _searchResults;
    int _selected = -1;
public:
    AssetSearchWidget(AssetType type = AssetType::none, size_t searchIncrement = 20);
    bool draw();
    const AssetID& currentSelected();
};


#endif //BRANEENGINE_ASSETSEARCHWIDGET_H
