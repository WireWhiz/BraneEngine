//
// Created by wirewhiz on 07/08/22.
//

#ifndef BRANEENGINE_ASSETSEARCHWIDGET_H
#define BRANEENGINE_ASSETSEARCHWIDGET_H

#include <string>
#include <mutex>
#include "assets/assetType.h"
#include "assets/assetID.h"

class AssetSearchWidget
{
    std::string _searchText;
    size_t _searchIncrement;
    AssetType _assetType;
    std::mutex _resultsLock;
   // std::vector<ServerFilesystem::SearchResult> _searchResults;
    int _selected = -1;
    bool _selectionMade = false;
    bool _fetchingSelected = false;
    void makeSelection();
public:
    AssetSearchWidget(AssetType type = AssetType::none, size_t searchIncrement = 20);
    bool draw();
    AssetID currentSelected();
};


#endif //BRANEENGINE_ASSETSEARCHWIDGET_H
