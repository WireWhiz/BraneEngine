//
// Created by wirewhiz on 20/07/22.
//

#include "createAssetWindow.h"
#include "../gltf/gltfLoader.h"
#include "../gltf/assemblyBuilder.h"


CreateAssetWindow::CreateAssetWindow(GUI &ui, GUIWindowID id) : GUIWindow(ui, id)
{

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
