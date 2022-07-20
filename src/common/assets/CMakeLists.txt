# src/common/assets
add_subdirectory(types)

add_library(assets_server STATIC
    assetID.cpp
    assetManager.cpp
    assetType.h
    assetType.cpp
    asset.cpp
    chunk.cpp
    assembly.cpp)

target_link_libraries(assets_server PUBLIC asset_type fileManager ecs systems ssl)
target_compile_definitions(assets_server PUBLIC SERVER)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(assets_client STATIC
        assetID.cpp
        assetManager.cpp
        assetType.h
        assetType.cpp
        asset.cpp
        chunk.cpp
        assembly.cpp)

target_link_libraries(assets_client PUBLIC asset_type fileManager ecs systems ssl graphics)
target_compile_definitions(assets_client PUBLIC CLIENT)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})