# src/common/assets
add_subdirectory(types)

add_library(assets STATIC 
  "assetID.cpp"
  "assetManager.cpp"
  "assetType.cpp"
  
   "types/componentAsset.h" "types/componentAsset.cpp" "types/assetTypes.h")

target_link_libraries(assets PUBLIC asset_type)
