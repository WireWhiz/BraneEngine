# src/common/assets
add_subdirectory(types)

find_package(OpenSSL REQUIRED)

set(ASSET_SOURCES
        assetID.cpp
        assetManager.cpp
        assetType.h
        assetType.cpp
        asset.cpp
        chunk.cpp
        assembly.cpp)

add_library(assets_server STATIC ${ASSET_SOURCES})
target_link_libraries(assets_server PUBLIC server_asset_types fileManager ecs systems utility OpenSSL::SSL BraneScriptRuntime)
target_compile_definitions(assets_server PUBLIC SERVER)

add_library(assets_client STATIC ${ASSET_SOURCES})
target_link_libraries(assets_client PUBLIC client_asset_types fileManager ecs systems utility OpenSSL::SSL graphics BraneScriptRuntime)
target_compile_definitions(assets_client PUBLIC CLIENT)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${robin_hood_INCLUDE_DIRS})
