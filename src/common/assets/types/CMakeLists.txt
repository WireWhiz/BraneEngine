# src/common/assets/types

set(ASSET_SOURCES
        "materialAsset.cpp"
        "meshAsset.cpp"
        "componentAsset.cpp"
        "shaderAsset.cpp"
        scriptAsset.cpp
        imageAsset.cpp)

add_library(client_asset_types STATIC ${ASSET_SOURCES})
target_link_libraries(client_asset_types PUBLIC graphics utility PRIVATE assets_client BraneScriptRuntime)
target_compile_definitions(client_asset_types PUBLIC CLIENT)

add_library(server_asset_types STATIC ${ASSET_SOURCES})
target_link_libraries(server_asset_types PUBLIC utility PRIVATE assets_server BraneScriptRuntime)
target_compile_definitions(server_asset_types PUBLIC SERVER)