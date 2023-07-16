set(CMAKE_FOLDER ${CMAKE_FOLDER}/Assets)
find_package(jsoncpp REQUIRED)
find_package(stb REQUIRED)

add_library(editorAssets STATIC
        gltfLoader.cpp
        assemblyBuilder.cpp
        editorAsset.cpp
        types/editorShaderAsset.cpp
        types/editorAssemblyAsset.cpp
        jsonVirtualType.cpp
        assetCache.cpp
        types/editorMaterialAsset.cpp
        types/editorChunkAsset.cpp
        assemblyReloadManager.cpp
        types/editorImageAsset.cpp)
target_link_libraries(editorAssets PUBLIC assets_client ecs jsoncpp::jsoncpp stb::stb)