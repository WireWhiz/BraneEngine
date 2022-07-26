//
// Created by wirewhiz on 21/07/22.
//

#include "virtualVariableWidgets.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "ecs/core/entity.h"

void VirtualVariableWidgets::displayVirtualComponentData(VirtualComponentView component)
{
    ImGui::PushID(component.description()->id);
    ImGui::Text("%s", component.description()->name.c_str());
    for(size_t i = 0; i < component.description()->members().size(); ++i)
    {
        auto& member = component.description()->members()[i];
        const std::string& name = component.description()->asset->memberNames()[i];
        if(!name.empty()){
            displayVirtualVariable(name.c_str(), member.type, component.data() + member.offset);
            ImGui::Spacing();
        }
    }
    ImGui::PopID();
}

void VirtualVariableWidgets::displayVirtualVariable(const char* name, VirtualType::Type type, byte* data)
{
    assert(name);
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
            if(ImGui::IsItemDeactivatedAfterEdit())
            {
                auto* em = Runtime::getModule<EntityManager>();
                em->tryGetEntity(id->id, *id);
            }
            ImGui::TextDisabled("(version %u)", id->version);
        }
            break;
        case VirtualType::virtualBool:
            ImGui::Checkbox(name, (bool*)data);
            break;
        case VirtualType::virtualInt:
            ImGui::InputScalar(name, ImGuiDataType_S32, (void*)data, NULL, NULL, "%d", 0);
            break;
        case VirtualType::virtualInt64:
            ImGui::InputScalar(name, ImGuiDataType_S64, (void*)data, NULL, NULL, "%d", 0);
            break;
        case VirtualType::virtualUInt:
            ImGui::InputScalar(name, ImGuiDataType_U32, (void*)data, NULL, NULL, "%u", 0);
            break;
        case VirtualType::virtualUInt64:
            ImGui::InputScalar(name, ImGuiDataType_U64, (void*)data, NULL, NULL, "%u", 0);
            break;
        case VirtualType::virtualFloat:
            ImGui::InputScalar(name, ImGuiDataType_Float, (void*)data, NULL, NULL, "%.3f", 0);
            break;
        case VirtualType::virtualString:
            ImGui::InputText(name, (std::string*)data);
            break;
        case VirtualType::virtualAssetID:
        {
            std::string id = ((AssetID*)data)->string();
            ImGui::InputText(name, &id);
            if(ImGui::IsItemDeactivatedAfterEdit())
                ((AssetID*)data)->parseString(id);
        }
            break;
        case VirtualType::virtualVec3:
            ImGui::InputFloat3(name, (float*)data);
            break;
        case VirtualType::virtualVec4:
            ImGui::InputFloat4(name, (float*)data);
            break;
        case VirtualType::virtualQuat:
            ImGui::InputFloat4(name, (float*)data);
            if(ImGui::IsItemDeactivatedAfterEdit())
                *(glm::quat*)data = glm::normalize(*(glm::quat*)data);
            break;
        case VirtualType::virtualMat4:
        {
            glm::mat4& mat = *(glm::mat4*)data;
            ImGui::InputFloat4("##MatValue1", &mat[0][0]);
            ImGui::InputFloat4("##MatValue2", &mat[1][0]);
            ImGui::InputFloat4("##MatValue3", &mat[2][0]);
            ImGui::InputFloat4("##MatValue4", &mat[3][0]);
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
    }
    ImGui::PopID();
}