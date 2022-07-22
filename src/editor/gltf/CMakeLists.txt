add_library(gltf STATIC gltfLoader.cpp assemblyBuilder.cpp)
target_link_libraries(gltf PUBLIC jsoncpp_static include_dir)