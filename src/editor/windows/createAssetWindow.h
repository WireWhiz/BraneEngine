//
// Created by wirewhiz on 20/07/22.
//

#ifndef BRANEENGINE_CREATEASSETWINDOW_H
#define BRANEENGINE_CREATEASSETWINDOW_H

#include <ui/guiWindow.h>
#include "../widgets/assetBrowserWidget.h"
#include "../gltf/assemblyBuilder.h"
#include "utility/asyncData.h"

class CreateAssetWindow : public GUIWindow
{
    struct AssetUploadContext {
        bool done = false;
        ServerDirectory* directory;
        std::string status;
        virtual void update() = 0;
        virtual ~AssetUploadContext() = default;
    };
    struct AssemblyUploadContext : public AssetUploadContext
    {
        std::unique_ptr<Assembly> assembly;
        std::atomic<size_t> meshesUploaded = 0;
        AsyncData<AssetID> assemblyID;
        bool uploadingAssembly = false;
        void update() override;
    };
    void loadAssetFromFile(const std::string &filename);
    bool _selectingDirectory = false;
    AssetBrowserWidget _browser;

    std::string _importFile = "";

    std::string _assetName = "new_asset";
    AssetType _assetType;

    std::unique_ptr<AssetUploadContext> _uploadContext;
public:
    CreateAssetWindow(GUI& ui, ServerDirectory* startingDir = nullptr);
    void draw() override;
};


#endif //BRANEENGINE_CREATEASSETWINDOW_H
