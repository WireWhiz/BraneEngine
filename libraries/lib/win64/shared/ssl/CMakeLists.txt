#lib/win64/shared/ssl

add_library(ssl INTERFACE)
target_link_libraries(ssl INTERFACE 
${CMAKE_CURRENT_SOURCE_DIR}/libcrypto.lib
${CMAKE_CURRENT_SOURCE_DIR}/libssl.lib
crypt32.lib
)