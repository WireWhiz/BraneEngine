
add_library(shaderCompiler STATIC shaderCompiler.cpp)
target_link_libraries(shaderCompiler PUBLIC include_dir shaderc shaderc_util glslc spirv-cross-cpp glslc robin_hood)