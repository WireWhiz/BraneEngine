# src/common/assets
add_subdirectory(types)

add_library(assets STATIC 
  "assetID.cpp"
  "assetManager.cpp"
   assetType.h assetType.cpp
  
   "types/componentAsset.h" "types/componentAsset.cpp" "types/assetTypes.h" "asset.h" "asset.cpp" chunk.cpp chunk.h assembly.cpp assembly.h nativeAsset.cpp nativeAsset.h)

target_link_libraries(assets PUBLIC asset_type fileManager ecs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})