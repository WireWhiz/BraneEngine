//
// Created by eli on 5/21/2022.
//

#include "dataWindow.h"
#include "editor/editor.h"
#include "editor/editorEvents.h"
#include "editor/assets/editorAsset.h"
#include "../widgets/virtualVariableWidgets.h"
#include <ui/gui.h>
#include <assets/assembly.h>
#include "assets/assetManager.h"
#include "assets/types/meshAsset.h"
#include <assets/assetID.h>
#include "common/ecs/entity.h"
#include "systems/transforms.h"
#include "assets/types/materialAsset.h"
#include "../widgets/assetSelectWidget.h"
#include "editor/assets/types/editorAssemblyAsset.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "editor/assets/types/editorMaterialAsset.h"
#include "editor/assets/assemblyReloadManager.h"
#include "ui/guiPopup.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

DataWindow::DataWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor)
{
    _name = "Data Inspector";
    ui.addEventListener<FocusAssetEvent>("focus asset", this, [this](const FocusAssetEvent* event){
        _focusedAsset = event->asset();
        _focusMode = FocusMode::asset;
        _focusedAssetEntity = -1;
    });
    ui.addEventListener<FocusEntityAssetEvent>("focus entity asset", this, [this](const FocusEntityAssetEvent* event){
        _focusedAssetEntity = event->entity();
    });
    ui.addEventListener<FocusEntityEvent>("focus entity", this, [this](const FocusEntityEvent* event){
        _focusedEntity = event->id();
        _focusMode = FocusMode::entity;
    });
}

void DataWindow::displayContent()
{
    switch(_focusMode)
    {
        case FocusMode::asset:
            displayAssetData();
            break;
        case FocusMode::entity:
            displayEntityData();
            break;
        default:
            ImGui::TextDisabled("No data to display");
    }
}

void DataWindow::displayAssetData()
{
    if(!_focusedAsset)
        return;

    ImGui::PushFont(_ui.fonts()[1]);
    ImGui::Text("%s%s", _focusedAsset->name().c_str(), (_focusedAsset->unsavedChanged()) ? " *" : "");
    ImGui::PopFont();
    ImGui::TextDisabled("%s", _focusedAsset->type().toString().c_str());
#ifdef NDEBUG
    try{
#endif
        switch(_focusedAsset->type().type())
        {
            case AssetType::shader:
                ImGui::Text("Source: %s", _focusedAsset->json()["source"].asCString());
                break;
            case AssetType::mesh:
                displayMeshData();
                break;
            case AssetType::assembly:
                displayAssemblyData();
                break;
            case AssetType::material:
                displayMaterialData();
                break;
            case AssetType::chunk:
                displayChunkData();
                break;
            default:
                ImGui::PushTextWrapPos();
                ImGui::Text("Asset Type %s not implemented yet. If you want to edit %s go to the GitHub and open an issue to put pressure on me.", _focusedAsset->type().toString().c_str(), _focusedAsset->name().c_str());
                ImGui::PopTextWrapPos();
        }
#ifdef NDEBUG
    }catch(const std::exception& e)
    {
        ImGui::TextColored({1,0,0,1}, "Error displaying asset: %s", e.what());
    }
#endif

}

void DataWindow::displayChunkData()
{
    ImGui::Text("LODs:");

    ImGui::Indent();
    int lodIndex = 0;
    auto& lods = _focusedAsset->json()["LODs"];
    int removedLOD = -1;
    for(auto& lod : lods)
    {
        ImGui::PushID(lodIndex);
        ImGui::TextDisabled("%d", lodIndex);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        int dragInt[2] = {lod["min"].asInt(), lod["max"].asInt()};
        if(ImGui::DragInt2("##Range", dragInt))
        {
            if(dragInt[0] < 0)
                dragInt[0] = 0;
            if(dragInt[1] < 0)
                dragInt[1] = 0;
            if(dragInt[0] > dragInt[1])
                dragInt[1] = dragInt[0];
            Json::Value newLod = lod;
            newLod["min"] = dragInt[0];
            newLod["max"] = dragInt[1];
            _focusedAsset->json().changeValue("LODs/" + std::to_string(lodIndex), newLod);
        }
        ImGui::SameLine();
        AssetID assembly(lods[lodIndex]["assembly"].asString());
        ImGui::SetNextItemWidth(160);
        if(AssetSelectWidget::draw(assembly, AssetType::assembly))
        {
            Json::Value newLod = lod;
            newLod["assembly"] = assembly.string();
            _focusedAsset->json().changeValue("LODs/" + std::to_string(lodIndex), newLod);
        }
        if(assembly.null())
        {
            ImGui::SameLine();
            if(ImGui::Button("Create Assembly"))
            {
                std::filesystem::path lodPath = _focusedAsset->file();
                lodPath.replace_filename(_focusedAsset->name() + "_LOD" + std::to_string(lodIndex) + ".assembly");

                auto* lodAssembly = new EditorAssemblyAsset(lodPath, _editor.project());

                Json::Value newLod = lod;
                newLod["assembly"] = lodAssembly->json()["id"];
                _focusedAsset->json().changeValue("LODs/" + std::to_string(lodIndex), newLod);
                lodAssembly->save();
                delete lodAssembly;
            }
        }
        if(lods.size() > 1)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 13);
            if(ImGui::Button("-", {15, 0}))
                removedLOD = lodIndex;
        }

        ImGui::PopID();
        ++lodIndex;
    }
    if(removedLOD != -1)
    {
        Json::Value newLODList;
        for(Json::ArrayIndex i = 0; i < lods.size(); ++i)
        {
            if(i != removedLOD)
                newLODList.append(lods[i]);
        }
        _focusedAsset->json().changeValue("LODs", newLODList);
    }

    ImGui::Unindent();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 13);
    if(ImGui::Button("+", {15, 0}))
    {
        Json::Value newLODList = _focusedAsset->json()["LODs"];
        Json::Value newLod;
        newLod["min"] = 0;
        newLod["max"] = 0;
        newLod["assembly"] = "null";
        newLODList.append(newLod);
        _focusedAsset->json().changeValue("LODs", newLODList);
    }
}

void DataWindow::displayAssemblyData()
{
    if(_focusedAssetEntity < _focusedAsset->json()["entities"].size())
    {
        displayEntityAssetData();
        return;
    }
    if(ImGui::CollapsingHeader("Dependencies")){
        ImGui::Indent();
        if(ImGui::CollapsingHeader("Materials")){
            ImGui::Indent();
            for(auto& cID : _focusedAsset->json()["dependencies"]["materials"])
            {
                ImGui::Selectable(cID.asCString());
            }
            ImGui::Unindent();
        }
        if(ImGui::CollapsingHeader("Meshes")){
            ImGui::Indent();
            for(auto& mID :  _focusedAsset->json()["dependencies"]["meshes"])
            {
                ImGui::Selectable(mID.asCString());
            }
            ImGui::Unindent();
        }
        ImGui::Unindent();
    }
    ImGui::Text("Entities: %u", _focusedAsset->json()["entities"].size());
    if(ImGui::IsItemHovered())
        ImGui::SetTooltip("Edit in entities window");
}

void DataWindow::displayMeshData()
{

}

class AddAssetComponentPopup : public GUIPopup
{
    AssetSearchWidget _search;
    std::shared_ptr<EditorAssemblyAsset> _focusedAsset;
    Json::ArrayIndex _focusedEntity;
    void drawBody() override
    {
        if(_search.draw())
        {
            auto asset = _focusedAsset;
            auto entity = _focusedEntity;
            Runtime::getModule<AssetManager>()->fetchAsset<ComponentAsset>(_search.currentSelected()).then([this, asset, entity](ComponentAsset* component){
                auto* compDef = Runtime::getModule<EntityManager>()->components().getComponentDef(component);
                VirtualComponent newComp(compDef);
                _focusedAsset->addEntityComponent(_focusedEntity, EditorAssemblyAsset::componentToJson(newComp));
            });
            ImGui::CloseCurrentPopup();
        }
    }
public:
    AddAssetComponentPopup(std::shared_ptr<EditorAssemblyAsset> focusedAsset, Json::ArrayIndex focusedEntity) :
        _focusedAsset(std::move(focusedAsset)), _focusedEntity(focusedEntity), _search(AssetType::component), GUIPopup("add component"){};

};

void DataWindow::displayEntityAssetData()
{
    auto* assembly = dynamic_cast<EditorAssemblyAsset*>(_focusedAsset.get());
    auto& entityAsset = _focusedAsset->json()["entities"][(Json::ArrayIndex)_focusedAssetEntity];
    ImGui::PushFont(_ui.fonts()[1]);
    std::string entityName = entityAsset["name"].asString();
    ImGui::InputText("##EntityName", &entityName);
    if(ImGui::IsItemDeactivatedAfterEdit())
        _focusedAsset->json().changeValue("entities/" + std::to_string(_focusedAssetEntity) + "/name", entityName);

    ImGui::PopFont();
    ImGui::TextDisabled("Index: %llu", _focusedAssetEntity);
    ImGui::Separator();
    int deleteComponent = -1;
    for (Json::ArrayIndex i = 0; i < entityAsset["components"].size(); ++i)
    {
        auto& component = entityAsset["components"][i];
        if(component["name"].empty())
            continue;
        ImGui::PushID(component["name"].asCString());
        bool displaying = ImGui::CollapsingHeader(component["name"].asCString(), ImGuiTreeNodeFlags_DefaultOpen);
        if(ImGui::BeginPopupContextItem("comp actions"))
        {
            bool transform = component["name"] == "transform";
            if(!transform)
            {
                if(ImGui::Selectable(ICON_FA_TRASH "delete"))
                    deleteComponent = i;
            }
            ImGui::EndPopup();
        }
        /*if(ImGui::BeginDragDropSource())
        {
            ImGui::TextDisabled("%s", component["name"].asCString());
            DraggedComponent dragData{(Assembly*)_focusedAsset->asset(), _focusedAssetEntity, i};
            ImGui::SetDragDropPayload("component", &dragData, sizeof(DraggedComponent));
            ImGui::EndDragDropSource();
        }
        if(ImGui::BeginDragDropTarget())
        {
            if(const ImGuiPayload* p = ImGui::AcceptDragDropPayload("component"))
            {
                auto dc = static_cast<DraggedComponent*>(p->Data);
                auto& srcComponents = dc->asset->entities[dc->entity].components;
                VirtualComponent source = srcComponents[dc->componentIndex];
                srcComponents.erase(srcComponents.begin() + dc->componentIndex);
                auto& destComponents = entityAsset.components;
                // The min is to account for if we're trying to move the component to the end, and just removed it from the same list.
                destComponents.insert(destComponents.begin() + std::min<uint32_t>(destComponents.size(), i), source);
            }
            ImGui::EndDragDropTarget();
        }*/
        if(displaying)
        {
            ImGui::Indent();
            if(component["name"].asString() == MeshRendererComponent::def()->name)
            {
                ImGui::Text("Mesh Index: %d", component["members"][0]["value"].asInt());
                for(Json::ArrayIndex mat = 0; mat < component["members"][1]["value"].size(); ++mat)
                {
                    ImGui::PushID(mat);
                    Json::ArrayIndex matIndex = component["members"][1]["value"][mat].asUInt();
                    ImGui::Text("Material %u (Assembly index %u)", mat, matIndex);
                    ImGui::SameLine();
                    AssetID matID(_focusedAsset->json()["dependencies"]["materials"].get(matIndex, "null").asString());
                    if(AssetSelectWidget::draw(matID, AssetType::material))
                    {
                        while(matIndex >= _focusedAsset->json()["dependencies"]["materials"].size())
                            _focusedAsset->json().data()["dependencies"]["materials"].append("null");

                        dynamic_cast<EditorAssemblyAsset*>(_focusedAsset.get())->changeMaterial(matIndex, matID);
                        _editor.reloadAsset(_focusedAsset);
                    }
                    ImGui::PopID();
                }
            }
            else
            {
                Json::Value data = component;
                auto res = VirtualVariableWidgets::displayAssetComponentData(data, _focusedAsset->json().data());
                if(res != UiChangeType::none)
                {
                    assembly->updateEntityComponent(_focusedAssetEntity, i, data, res != UiChangeType::finished);
                    if(res == UiChangeType::finished)
                    {

                        _editor.reloadAsset(_focusedAsset);
                    }
                }
            }
            ImGui::Unindent();
        }
        ImGui::PopID();
    }
    if(ImGui::Button("Add Component", {ImGui::GetContentRegionAvail().x, 0}))
    {
        _ui.openPopup(std::make_unique<AddAssetComponentPopup>(std::dynamic_pointer_cast<EditorAssemblyAsset>(_focusedAsset), _focusedAssetEntity));
    }

    if(deleteComponent != -1)
        assembly->removeEntityComponent(_focusedAssetEntity, deleteComponent);
}

void DataWindow::displayEntityData()
{

    ImGui::Text("Entity ID: %u", _focusedEntity.id);
    ImGui::TextDisabled("Version: %u", _focusedEntity.version);
    ImGui::Separator();
    auto* em = Runtime::getModule<EntityManager>();
    if(!em->entityExists(_focusedEntity))
    {
        ImGui::TextColored({1,0,0,1}, "Entity Destroyed!");
        return;
    }

    auto& components = em->getEntityArchetype(_focusedEntity)->components();
    for (auto cid : components)
    {
        if((int)VirtualVariableWidgets::displayVirtualComponentData(em->getComponent(_focusedEntity, cid)) > 0)
            em->markComponentChanged(_focusedEntity, cid);
        ImGui::Separator();
    }
}

void DataWindow::displayMaterialData()
{
    auto* material = static_cast<EditorMaterialAsset*>(_focusedAsset.get());
    AssetID vertexID(material->json()["vertexShader"].asString());
    if(AssetSelectWidget::draw(vertexID, AssetType::shader))
    {
        material->json().changeValue("vertexShader", vertexID.string());
        _editor.reloadAsset(_focusedAsset);
    };
    ImGui::SameLine();
    ImGui::Text("Vertex Shader");
    AssetID fragmentID(material->json()["fragmentShader"].asString());
    if(AssetSelectWidget::draw(fragmentID, AssetType::shader))
    {
        material->json().changeValue("fragmentShader", fragmentID.string());
        _editor.reloadAsset(_focusedAsset);
    }
    ImGui::SameLine();
    ImGui::Text("Fragment Shader");
    ImGui::Spacing();
    ImGui::TextDisabled("Custom property components coming eventually");
}







