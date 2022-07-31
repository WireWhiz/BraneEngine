//
// Created by wirewhiz on 20/07/22.
//

#include "createAssetWindow.h"
#include "../gltf/gltfLoader.h"
#include "../gltf/assemblyBuilder.h"
#include "../serverFilesystem.h"
#include "fileManager/fileManager.h"
#include "../gltf/gltfLoader.h"
#include "../gltf/assemblyBuilder.h"

CreateAssetWindow::CreateAssetWindow(GUI &ui, ServerDirectory* startingDir) : GUIWindow(ui), _browser(ui, false)
{
    _name = "Create Asset";
    if(startingDir)
        _browser.setDirectory(startingDir);
}

void CreateAssetWindow::loadAssetFromFile(const std::string &filename)
{
    std::string suffix = filename.substr(filename.find_last_of('.'));
    for (size_t i = 0; i < suffix.size(); ++i)
        suffix[i] = std::tolower(suffix[i]);
    if(suffix == ".glb")
    {
        gltfLoader loader;
       /* loader.loadGlbFromString(assetData);
        auto assembly = AssetBuilder::buildAssembly(assetName, loader);

        //Save meshes
        for(auto& mesh : assembly.meshes)
        {
            std::string meshFilename = assetPath + assetName + "_meshes/" + mesh->name + ".mesh";

            AssetInfo info{0, meshFilename, mesh->name, AssetType::mesh};
            _db.insertAssetInfo(info);
            mesh->id.serverAddress = Config::json()["network"]["domain"].asString();
            mesh->id.id = info.id;
            assembly.assembly->meshes.push_back(mesh->id);
            _fm.writeAsset(mesh.get(), assetDirectory + "/" + meshFilename);
        }

        //Save assembly
        std::string assemblyFilename = assetPath + assetName + ".assembly";
        AssetInfo info{0, assemblyFilename, assetName, AssetType::assembly};
        _db.insertAssetInfo(info);
        assembly.assembly->id.serverAddress = Config::json()["network"]["domain"].asString();
        assembly.assembly->id.id = info.id;
        _fm.writeAsset(assembly.assembly.get(), assetDirectory + "/" +assemblyFilename);*/
    }
}

void CreateAssetWindow::displayContent()
{
    if(_uploadContext && !_uploadContext->done)
        ImGui::BeginDisabled();
    const std::array<const char*, 7> types = {"select type", "component","system", "mesh", "texture", "shader", "assembly"};
    ImGui::InputText("Name", &_assetName, ImGuiInputTextFlags_AutoSelectAll);
    if(ImGui::BeginCombo("Type", _assetType.toString().c_str()))
    {
        for (size_t i = 1; i < types.size(); ++i)
        {
            if(ImGui::Selectable(types[i]))
                _assetType.set(types[i]);
        }
        ImGui::EndCombo();
    }

    if(!_selectingDirectory)
    {
        if(ImGui::Button("change"))
            _selectingDirectory = true;
        ImGui::SameLine();
        ImGui::Text("Directory: %s", _browser.currentDirectory()->path().c_str());
    }
    else
    {
        ImGui::BeginChild("Directory View", {ImGui::GetWindowContentRegionWidth(), 500}, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        if(ImGui::Button("select"))
            _selectingDirectory = false;
        ImGui::SameLine();
        _browser.displayFullBrowser();
        ImGui::EndChild();
    }
    ImGui::Separator();
    bool noCreate = _assetType == AssetType::Type::none || _selectingDirectory || (_uploadContext && !_uploadContext->done);
    switch(_assetType.type())
    {
        case AssetType::assembly:
            if(ImGui::Button("Select gltf"))
                _importFile = FileManager::requestLocalFilePath("gltf", {"*.glb", "*.gltf"});
            ImGui::SameLine();
            ImGui::Text("Import from file: %s", _importFile.c_str());
            if(_importFile.empty())
                noCreate = true;
            break;
        default:
            noCreate = true;
            ImGui::Text("Asset type %s not implemented", _assetType.toString().c_str());
    }
    ImGui::Separator();
    if(_uploadContext && !_uploadContext->done)
        ImGui::EndDisabled();
    if(noCreate)
        ImGui::BeginDisabled();
    if(ImGui::Button("Create"))
    {
        auto* fs = Runtime::getModule<ServerFilesystem>();
        switch(_assetType.type())
        {
            case AssetType::assembly:
            {
                gltfLoader loader;
                if(!loader.loadFromFile(_importFile))
                {
                    Runtime::error("Failed to read gltf file!");
                    break;
                }
                AssemblyBuilder::AssemblyAssets assets;
                try{
                    assets = AssemblyBuilder::buildAssembly(_assetName, loader);
                }
                catch(const std::exception& e)
                {
                    Runtime::error("Failed to parse gltf file! " + std::string(e.what()));
                    break;
                }

                _uploadContext = std::make_unique<AssemblyUploadContext>();
                _uploadContext->directory = _browser.currentDirectory();
                assets.assembly->meshes.resize(assets.meshes.size());
                for(size_t i = 0; i < assets.meshes.size(); ++i)
                {
                    fs->saveAsset(_browser.currentDirectory(), assets.meshes[i].get()).then([this, i](AssetID id){
                        ((AssemblyUploadContext*)_uploadContext.get())->assembly->meshes[i] = id.string();
                        ((AssemblyUploadContext*)_uploadContext.get())->meshesUploaded++;
                    });
                }
                ((AssemblyUploadContext*)_uploadContext.get())->assembly = std::move(assets.assembly);
                break;
            }
            default:
                noCreate = true;
                ImGui::Text("Asset type %s not implemented", _assetType.toString().c_str());
        }
    }
    if(noCreate)
        ImGui::EndDisabled();
    if(_uploadContext)
    {
        _uploadContext->update();
        if(!_uploadContext->done)
            ImGui::Text("Upload status: %s", _uploadContext->status.c_str());
        else
            ImGui::Text("Upload completed");
    }
}

void CreateAssetWindow::AssemblyUploadContext::update()
{
    if(!uploadingAssembly && meshesUploaded == assembly->meshes.size())
    {
        uploadingAssembly = true;
        auto* fs = Runtime::getModule<ServerFilesystem>();
        fs->saveAsset(directory, assembly.get()).then([this](AssetID id){
            done = true;
        });
        status = "Uploading assembly";
    }
    else if(!uploadingAssembly)
    {
        status = "Uploading meshes (" + std::to_string(meshesUploaded) + "/" + std::to_string(assembly->meshes.size()) + ")";
    }

}
