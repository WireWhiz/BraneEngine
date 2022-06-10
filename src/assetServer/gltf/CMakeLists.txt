add_executable(gltfLoader
        main.cpp
        )

target_link_libraries(gltfLoader PUBLIC gltf assets)

add_library(gltf STATIC gltfLoader.cpp gltfLoader.h)
target_link_libraries(gltf PUBLIC json include_dir)
file(COPY adamHead DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)