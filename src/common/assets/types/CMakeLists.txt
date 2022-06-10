# src/common/assets/types
add_library(asset_type STATIC
 "materialAsset.cpp"
 "meshAsset.cpp"
 "componentAsset.cpp"
 "shaderAsset.h" "shaderAsset.cpp" scriptAsset.cpp scriptAsset.h "componentAsset.h" "componentAsset.cpp" "assetTypes.h" )

target_link_libraries(asset_type PUBLIC include_dir)