//
// Created by wirewhiz on 07/08/22.
//

#include "assetSearchWidget.h"
#include "runtime/runtime.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "assets/assetManager.h"

AssetSearchWidget::AssetSearchWidget(AssetType type, size_t searchIncrement)
{
    _assetType = type;
    _searchIncrement = searchIncrement;
    /*_fm = Runtime::getModule<ServerFilesystem>();
    _fm->searchAssets(0, _searchIncrement, "", _assetType).then([this](auto results){
       std::scoped_lock lock(_resultsLock);
        _searchResults = results;
    });*/
}

bool AssetSearchWidget::draw()
{
    /*ImGui::BeginDisabled(_selectionMade);
    if(ImGui::InputText("search", &_searchText))
    {
        _selected = -1;
        _searchResults.resize(0);
        _fm->searchAssets(0, _searchIncrement, _searchText, _assetType).then([this](auto results){
            std::scoped_lock lock(_resultsLock);
            _searchResults = results;
        });
    }
    ImGui::Separator();
    ImGui::BeginChild("asset search", {ImGui::GetContentRegionAvail().x, 500});

    std::scoped_lock lock(_resultsLock);
    for(size_t i = 0; i < _searchResults.size(); ++i)
    {
        auto& r = _searchResults[i];
        if(ImGui::Selectable(r.name.c_str(), _selected == i, ImGuiSelectableFlags_DontClosePopups))
            _selected = i;
        if(ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("ID: %s", r.id.string().c_str());
            if(ImGui::IsMouseDoubleClicked(0))
                makeSelection();
        }
    }
    // Not perfect, will still show button if the total number of search results is a multiple of search increment
    if(_searchResults.size() % _searchIncrement == 0)
    {
        if(ImGui::Selectable("Load More"))
        {
            _fm->searchAssets(_searchResults.size(), _searchIncrement, _searchText, _assetType).then([this](auto results){
                std::scoped_lock lock(_resultsLock);
                _searchResults.insert(_searchResults.begin(), results.begin(), results.end());
            });
        }
    }
    ImGui::EndChild();
    ImGui::Separator();
    ImGui::BeginDisabled(_selected < 0);
    if(ImGui::Button("Select"))
        makeSelection();

    ImGui::EndDisabled();
    ImGui::EndDisabled();

    return _selectionMade && !_fetchingSelected;*/
	return false;
}

AssetID AssetSearchWidget::currentSelected()
{
	/*std::scoped_lock lock(_resultsLock);
	return _searchResults[_selected].id;*/
	return {};
}

void AssetSearchWidget::makeSelection()
{
	/*_selectionMade = true;
    _fetchingSelected = true;
    auto* am = Runtime::getModule<AssetManager>();
    auto selected = _searchResults[_selected].id;
    if(am->hasAsset(selected))
        _fetchingSelected = false;
    else
        am->fetchAsset<Asset>(selected).then([this](Asset* asset){
            _fetchingSelected = false;
        });*/

}
