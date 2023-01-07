//
// Created by wirewhiz on 07/08/22.
//

#ifndef BRANEENGINE_ASSETSELECTWIDGET_H
#define BRANEENGINE_ASSETSELECTWIDGET_H

#include "assetSearchWidget.h"
#include "assets/assetType.h"
#include <memory>

class AssetID;

class AssetSelectWidget {
    static std::unique_ptr<AssetSearchWidget> _searchWidget;

public:
    static bool draw(AssetID &id, AssetType type = AssetType::none);
};

#endif // BRANEENGINE_ASSETSELECTWIDGET_H
