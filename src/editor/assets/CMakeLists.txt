set(CMAKE_FOLDER ${CMAKE_FOLDER}/Assets)
add_library(editorAssets STATIC
        gltfLoader.cpp
        assemblyBuilder.cpp
        editorAsset.cpp
        types/editorShaderAsset.cpp
        types/editorAssemblyAsset.cpp
        jsonVirtualType.cpp assetCache.cpp assetCache.h types/editorMaterialAsset.cpp types/editorMaterialAsset.h)
target_link_libraries(editorAssets PUBLIC jsoncpp_static include_dir)