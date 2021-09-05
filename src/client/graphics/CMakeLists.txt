#src/client/graphics

add_library(graphics STATIC
	graphics.cpp
	graphicsDevice.cpp
	graphicsPipeline.cpp
	material.cpp
	mesh.cpp
	renderer.cpp
	shader.cpp
	swapChain.cpp
	texture.cpp
	window.cpp
)
target_link_libraries(graphics PRIVATE utility)
target_compile_definitions(graphics PUBLIC GLFW_INCLUDE_VULKAN ${COMMON_DEFS})