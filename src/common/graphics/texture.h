#pragma once
#include "graphicsDevice.h"
#include "graphicsBuffer.h"

#include <vulkan/vulkan.h>
#include <stb/stb_image.h>

#include <string>
#include <array>

namespace graphics
{
	typedef uint64_t TextureID;
	extern const std::array<const char*, 1> textureFileExtensions;
	class Texture
	{

		VkImage _textureImage;
		VkDeviceMemory _textureImageMemory;

		VkImageView _textureImageView;
		VkSampler _sampler = VK_NULL_HANDLE;

		uint32_t width;
		uint32_t height;

		void createTextureImageView();
		void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(GraphicsBuffer& buffer);
		void loadFromPixels(const unsigned char* pixels, uint32_t width, uint32_t height);

	public:
		Texture(const std::string& filename);
		Texture(TextureID id);
		Texture(const unsigned char* pixels, uint32_t width, uint32_t height);
		~Texture();
		VkImage get();
		VkImageView view();
		VkSampler sampler();
	};
}