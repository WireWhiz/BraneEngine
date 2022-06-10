# lib/unix/ssl
add_library(ssl INTERFACE)
target_link_libraries(ssl INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/libssl.a
	${CMAKE_CURRENT_SOURCE_DIR}/libcrypto.a
	-ldl
	-lpthread
)