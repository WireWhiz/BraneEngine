//
// Created by wirewhiz on 21/07/22.
//

#include "virtualVariableWidgets.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "common/ecs/entity.h"
#include "systems/transforms.h"

UiChangeType VirtualVariableWidgets::displayVirtualComponentData(VirtualComponentView component)
{
    UiChangeType changed = UiChangeType::none;
    ImGui::PushID(component.description()->id);
    bool viewOnly = component.description() == Transform::def()
            || component.description() == LocalTransform::def()
            || component.description() == Children::def();
    ImGui::BeginDisabled(viewOnly);
    for(size_t i = 0; i < component.description()->members().size(); ++i)
    {
        auto& member = component.description()->members()[i];
        const std::string& name = component.description()->asset->memberNames()[i];
        if(!name.empty()){
            changed = (UiChangeType)std::max((uint8_t)changed, (uint8_t)displayVirtualVariable(name.c_str(), member.type, component.data() + member.offset));
            ImGui::Spacing();
        }
    }
    ImGui::EndDisabled();
    ImGui::PopID();
    return changed;
}

UiChangeType VirtualVariableWidgets::displayVirtualVariable(const char* name, VirtualType::Type type, byte* data)
{
    assert(name);
    const float dragSpeed = 0.05;
    UiChangeType changed = UiChangeType::none;
    ImGui::PushID(name);
    switch(type)
    {
        case VirtualType::virtualUnknown:
            ImGui::Text("Can't edit virtual unknowns, how are you seeing this?");
            break;
        case VirtualType::virtualEntityID:
        {

            auto* id = (EntityID*)data;
            ImGui::InputScalar(name, ImGuiDataType_U32, (void*) &id->id, NULL, NULL, "%u", 0);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
            {
                auto* em = Runtime::getModule<EntityManager>();
                em->tryGetEntity(id->id, *id);
                changed = UiChangeType::finished;
            }
            ImGui::TextDisabled("(version %u)", id->version);
        }
            break;
        case VirtualType::virtualBool:
            ImGui::Checkbox(name, (bool*)data);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualInt:
            ImGui::DragInt(name, (int*)data, 1);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualInt64:
            ImGui::InputScalar(name, ImGuiDataType_S64, (void*)data, NULL, NULL, "%d", 0);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualUInt:
            ImGui::InputScalar(name, ImGuiDataType_U32, (void*)data, NULL, NULL, "%u", 0);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualUInt64:
            ImGui::InputScalar(name, ImGuiDataType_U64, (void*)data, NULL, NULL, "%u", 0);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualFloat:
            ImGui::DragFloat(name, (float*)data, dragSpeed);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualString:
            ImGui::InputText(name, (std::string*)data);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualAssetID:
        {
            std::string id = ((AssetID*)data)->string();
            ImGui::InputText(name, &id);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            if(ImGui::IsItemDeactivatedAfterEdit())
                ((AssetID*)data)->parseString(id);
        }
            break;
        case VirtualType::virtualVec3:
            ImGui::DragFloat3(name, (float*)data, dragSpeed);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualVec4:
            ImGui::DragFloat4(name, (float*)data,dragSpeed);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            break;
        case VirtualType::virtualQuat:
            ImGui::DragFloat4(name, (float*)data, dragSpeed);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            if(ImGui::IsItemDeactivatedAfterEdit())
                *(glm::quat*)data = glm::normalize(*(glm::quat*)data);
            break;
        case VirtualType::virtualMat4:
        {
            glm::mat4& mat = *(glm::mat4*)data;
            ImGui::InputFloat4("##MatValue1", &mat[0][0]);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            ImGui::InputFloat4("##MatValue2", &mat[1][0]);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            ImGui::InputFloat4("##MatValue3", &mat[2][0]);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
            ImGui::InputFloat4("##MatValue4", &mat[3][0]);
            if(ImGui::IsItemEdited())
                changed = UiChangeType::ongoing;
            if(ImGui::IsItemDeactivatedAfterEdit())
                changed = UiChangeType::finished;
        }
            break;
        case VirtualType::virtualFloatArray:
        {
            inlineFloatArray& array = *(inlineFloatArray*)data;
            ImGui::Text("%s (TODO: Array editing)", name);
            for(auto f : array)
            {
                ImGui::TextDisabled("%f", f);
            }
        }
            break;
        case VirtualType::virtualIntArray:
        {
            inlineIntArray& array = *(inlineIntArray*)data;
            ImGui::Text("%s (TODO: Array editing)", name);
            for(auto f : array)
            {
                ImGui::TextDisabled("%d", f);
            }
        }
            break;
        case VirtualType::virtualUIntArray:
        {
            inlineUIntArray& array = *(inlineUIntArray*)data;
            ImGui::Text("%s (TODO: Array editing)", name);
            for(auto f : array)
            {
                ImGui::TextDisabled("%u", f);
            }
        }
            break;
        case VirtualType::virtualEntityIDArray:
        {
            inlineEntityIDArray& array = *(inlineEntityIDArray *)data;
            ImGui::Text("%s (TODO: Array editing)", name);
            for(auto id : array)
            {
                ImGui::TextDisabled("%u", id.id);
            }
        }
            break;
    }
    ImGui::PopID();
    return changed;
}