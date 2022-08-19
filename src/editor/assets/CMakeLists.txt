set(CMAKE_FOLDER ${CMAKE_FOLDER}/GLTF)
add_library(editorAssets STATIC gltfLoader.cpp assemblyBuilder.cpp editorAsset.cpp types/editorShaderAsset.cpp types/editorShaderAsset.h)
target_link_libraries(editorAssets PUBLIC jsoncpp_static include_dir)