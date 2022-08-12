# src/common/assets/types

set(ASSET_SOURCES
        "materialAsset.cpp"
        "meshAsset.cpp"
        "componentAsset.cpp"
        "shaderAsset.h"
        "shaderAsset.cpp"
        scriptAsset.cpp
        "componentAsset.h"
        "componentAsset.cpp")

add_library(client_asset_types STATIC ${ASSET_SOURCES})
target_link_libraries(client_asset_types PUBLIC include_dir graphics)
target_compile_definitions(client_asset_types PUBLIC CLIENT)

add_library(server_asset_types STATIC ${ASSET_SOURCES})
target_link_libraries(server_asset_types PUBLIC include_dir)
target_compile_definitions(server_asset_types PUBLIC SERVER)