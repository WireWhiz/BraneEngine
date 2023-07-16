find_package(glslang REQUIRED)
find_package(spirv-cross REQUIRED)
find_package(shaderc REQUIRED)
find_package(robin_hood REQUIRED)

add_library(shaderCompiler STATIC shaderCompiler.cpp)
target_link_libraries(shaderCompiler PUBLIC shaderc::shaderc spirv-cross::spirv-cross-cpp glslang::glslang)
target_include_directories(shaderCompiler PUBLIC ${robin_hood_INCLUDE_DIR})