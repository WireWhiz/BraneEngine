# src/common/assets
add_subdirectory(types)

set(ASSET_SOURCES
        assetID.cpp
        assetManager.cpp
        assetType.h
        assetType.cpp
        asset.cpp
        chunk.cpp
        assembly.cpp)

add_library(assets_server STATIC ${ASSET_SOURCES})

target_link_libraries(assets_server PUBLIC sqlite3 server_asset_types fileManager ecs systems ssl)
target_compile_definitions(assets_server PUBLIC SERVER)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(assets_client STATIC ${ASSET_SOURCES})

target_link_libraries(assets_client PUBLIC client_asset_types fileManager ecs systems ssl graphics)
target_compile_definitions(assets_client PUBLIC CLIENT)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})