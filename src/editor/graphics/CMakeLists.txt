
add_library(shaderCompiler STATIC shaderCompiler.cpp)
target_link_libraries(shaderCompiler PUBLIC include_dir shaderc spirv-cross-cpp)