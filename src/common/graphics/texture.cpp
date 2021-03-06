#include "texture.h"

namespace graphics
{
    const std::array<const char*, 1> textureFileExtensions = {".png" };

    void Texture::createTextureImageView()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _textureImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device->get(), &viewInfo, nullptr, &_textureImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void Texture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {

        SingleUseCommandBuffer cmdBuffer(device->graphicsPool());

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = _textureImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            cmdBuffer.get(),
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );


        cmdBuffer.submit(device->graphicsQueue());
    }

    void Texture::copyBufferToImage(GraphicsBuffer& buffer)
    {
        SingleUseCommandBuffer cmdBuffer(device->transferPool());

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            cmdBuffer.get(),
            buffer.get(),
            _textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        cmdBuffer.submit(device->transferQueue());
    }

    Texture::Texture(const std::string& filename)
    {
        int w, h, texChannels;
        unsigned char* pixels = stbi_load(filename.c_str(), &w, &h, &texChannels, STBI_rgb_alpha);
        if (pixels == nullptr)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        loadFromPixels(pixels, w, h);
        stbi_image_free(pixels);
    }
    Texture::Texture(TextureID id) : Texture("textures/" + std::to_string(id) + ".png")
    {
        
    }
    void Texture::loadFromPixels(const unsigned char* pixels, uint32_t width, uint32_t height)
    {
        this->width = width;
        this->height = height;
        VkDeviceSize imageSize = width * height * 4;

        GraphicsBuffer stagingBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.setData(pixels, imageSize, 0);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;

        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        if (vkCreateImage(device->get(), &imageInfo, nullptr, &_textureImage) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device->get(), _textureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device->get(), &allocInfo, nullptr, &_textureImageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate texture memory!");
        }

        vkBindImageMemory(device->get(), _textureImage, _textureImageMemory, 0);

        transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer);
        transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        createTextureImageView();
    }
    Texture::Texture(const unsigned char* pixels, uint32_t width, uint32_t height)
    {
        loadFromPixels(pixels, width, height);
	}
    Texture::~Texture()
    {
        if(_sampler != VK_NULL_HANDLE)
            vkDestroySampler(device->get(), _sampler, nullptr);
        vkDestroyImageView(device->get(), _textureImageView, nullptr);
        vkDestroyImage(device->get(), _textureImage, nullptr);
        vkFreeMemory(device->get(), _textureImageMemory, nullptr);
    }

    VkImage Texture::get()
    {
        return _textureImage;
    }

    VkImageView Texture::view()
    {
        return _textureImageView;
    }

    VkSampler Texture::sampler()
    {
        if (_sampler == VK_NULL_HANDLE)
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = device->properties().limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 0.0f;

            if (vkCreateSampler(device->get(), &samplerInfo, nullptr, &_sampler) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
        return _sampler;
    }
}