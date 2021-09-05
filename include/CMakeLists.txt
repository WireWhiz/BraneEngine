# file: include

file(GLOB_RECURSE asmjit_sources asmjit/*.cpp)
add_library(asmjit STATIC ${asmjit_sources})
target_include_directories(asmjit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(asmjit PUBLIC ASMJIT_STATIC)
if (UNIX)
target_link_libraries(asmjit PUBLIC -lrt -lpthread)
endif(UNIX)

set(INCS ${includes} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)