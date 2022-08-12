//
// Created by wirewhiz on 07/08/22.
//

#include "assetSelectWidget.h"
#include "assets/assetID.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "assets/assetManager.h"
#include "runtime/runtime.h"

std::unique_ptr<AssetSearchWidget> AssetSelectWidget::_searchWidget;

bool AssetSelectWidget::draw(AssetID* id, AssetType type)
{
    bool changed = false;
    ImGui::PushID(id);
    std::string name;
    if(id->serverAddress.empty())
        name = "none";
    else
        name = Runtime::getModule<AssetManager>()->getAsset<Asset>(*id)->name;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.2);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.1, 0.1, 0.1, 1});
    ImGui::InputText(("##" + name).c_str(), &name, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    if(ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseDoubleClicked(0))
        {
            _searchWidget = std::make_unique<AssetSearchWidget>(type);
            ImGui::OpenPopup("select asset");
        }
        if(ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            id->id = 0;
            id->serverAddress = "";
            changed = true;
        }
    }
    if(ImGui::BeginPopup("select asset"))
    {
        if(_searchWidget->draw())
        {
            auto* am = Runtime::getModule<AssetManager>();
            *id = _searchWidget->currentSelected();
            changed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    //TODO drag drop target for dragging from asset browser
    ImGui::PopID();
    return changed;
}
