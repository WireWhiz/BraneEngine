# src/common/assets
add_subdirectory(types)

add_library(assets STATIC 
    "assetID.cpp"
    "assetManager.cpp"
    assetType.h
    assetType.cpp
  
   "asset.h" "asset.cpp" chunk.cpp chunk.h assembly.cpp assembly.h ../systems/transforms.cpp ../systems/transforms.h)

target_link_libraries(assets PUBLIC asset_type fileManager ecs systems)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})