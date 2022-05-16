#include "material.h"

namespace graphics
{
    void Material::buildDescriptorSetVars(SwapChain* swapChain)
    {
        if (_descriptorSetLayout != VK_NULL_HANDLE)
            return;
        
        //Create layout
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        
        if (_textures.size() > 0)
        {
            for (size_t i = 1; i <= _textures.size(); i++)
            {
                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = i;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

                bindings.push_back(samplerLayoutBinding);
            }
        }
        
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device->get(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        //Create pool
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain->size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChain->size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        poolInfo.maxSets = static_cast<uint32_t>(swapChain->size());

        if (vkCreateDescriptorPool(device->get(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
        
        
    }

    Material::~Material()
    {
        vkDestroyDescriptorSetLayout(device->get(), _descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(device->get(), _descriptorPool, nullptr);

        vkDestroyPipeline(device->get(), _pipeline, nullptr);
        vkDestroyPipelineLayout(device->get(), _pipelineLayout, nullptr);
    }

    
    /*ComponentAsset Material::createUniformComponent(ComponentID id, const std::vector<VirtualType> vars, size_t alignment)
    {
        assert(_id == ULONG_MAX && "createUniformComponent can only be called once");
        std::vector<NativeVarDef> varDefs(vars.size());
        size_t index = 0;
        for (size_t i = 0; i < vars.size(); i++)
        {
            varDefs[i].type = vars[i];
            varDefs[i].index = index;
            index += sizeofVirtual(vars[i]);
            if (index % 16 != 0)
                index += 16 - (index % 16);
        }

        ComponentAsset compDef(vars.size(), id);
        compDef.initalize(varDefs, index);
        _id = compDef.id();

        return compDef;
    }
    ComponentID Material::componentID()
    {
        return _id;
    }*/
    void Material::addTextureDescriptor(Texture* texture)
    {
        assert(_descriptorSetLayout == VK_NULL_HANDLE && "Can only edit materials before they are used, make a new one instead");
        _textures.push_back(texture);
    }
    void Material::setGeometry(Shader* shader)
	{
        assert(_descriptorSetLayout == VK_NULL_HANDLE && "Can only edit materials before they are used, make a new one instead");
        assert(_descriptorSetLayout == VK_NULL_HANDLE);
        assert(shader->type() == VK_SHADER_STAGE_GEOMETRY_BIT);
		_geometryShader = shader;
	}
	void Material::setVertex(Shader* shader)
	{
        assert(_descriptorSetLayout == VK_NULL_HANDLE && "Can only edit materials before they are used, make a new one instead");
        assert(shader->type() == VK_SHADER_STAGE_VERTEX_BIT);
		_vertexShader = shader;
	}
	void Material::setFragment(Shader* shader)
	{
        assert(_descriptorSetLayout == VK_NULL_HANDLE && "Can only edit materials before they are used, make a new one instead");
        assert(shader->type() == VK_SHADER_STAGE_FRAGMENT_BIT);
		_fragmentShader = shader;
	}
	void Material::buildGraphicsPipeline(SwapChain* swapChain)
	{
        if (_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device->get(), _pipeline, nullptr);
            vkDestroyPipelineLayout(device->get(), _pipelineLayout, nullptr);
        }
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)_bindings.size();
		vertexInputInfo.pVertexBindingDescriptions = _bindings.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)_attributes.size();
        vertexInputInfo.pVertexAttributeDescriptions = _attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChain->extent().width;
        viewport.height = (float)swapChain->extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChain->extent();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;


        VkPushConstantRange push_constant{};
        push_constant.offset = 0;
        push_constant.size = sizeof(MeshPushConstants);
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        buildDescriptorSetVars(swapChain);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;

        if (vkCreatePipelineLayout(device->get(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        if (_geometryShader != nullptr)
            shaderStages.push_back(_geometryShader->stageInfo());
        if (_vertexShader != nullptr)
            shaderStages.push_back(_vertexShader->stageInfo());
        if (_fragmentShader != nullptr)
            shaderStages.push_back(_fragmentShader->stageInfo());


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional

        pipelineInfo.layout = _pipelineLayout;

        pipelineInfo.renderPass = swapChain->renderPass();
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(device->get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        buildDescriptorSetVars(swapChain);
	}
    void Material::getImageDescriptors(std::vector<VkWriteDescriptorSet>& descriptors, std::vector<VkDescriptorImageInfo>& imageInfo, VkDescriptorSet& descriptorSet)
    {
        for (size_t i = 0; i < _textures.size(); i++)
        {
            VkDescriptorImageInfo newImageInfo{};
            newImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            newImageInfo.imageView = _textures[i]->view();
            newImageInfo.sampler = _textures[i]->sampler();
            imageInfo.push_back(newImageInfo);

            VkWriteDescriptorSet newSet{};
            newSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            newSet.dstSet = descriptorSet;
            newSet.dstBinding = i + 1;
            newSet.dstArrayElement = 0;
            newSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            newSet.descriptorCount = 1;
            newSet.pImageInfo = &imageInfo[imageInfo.size() - 1];
            descriptors.push_back(newSet);
        }
        
    }
    VkPipeline Material::pipeline()
    {
        assert(_pipeline != VK_NULL_HANDLE);
        return _pipeline;
    }
    VkPipelineLayout Material::pipelineLayout()
    {
        assert(_pipelineLayout != VK_NULL_HANDLE);
        return _pipelineLayout;
    }
    VkDescriptorSetLayout Material::descriptorLayout()
    {

        assert(_descriptorSetLayout != VK_NULL_HANDLE);
        return _descriptorSetLayout;
    }
    VkDescriptorPool Material::descriptorPool()
    {
        assert(_descriptorPool != VK_NULL_HANDLE);
        return _descriptorPool;
    }

	void Material::addBinding(uint32_t index, uint32_t stride)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = index;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		_bindings.push_back(bindingDescription);
	}

	void Material::addAttribute(uint32_t binding, VkFormat format, uint32_t offset)
	{
		VkVertexInputAttributeDescription attributeDescription;
		attributeDescription.binding = binding;
		attributeDescription.location = _attributes.size();
		attributeDescription.format = format;
		attributeDescription.offset = offset;
		_attributes.push_back(attributeDescription);
	}
}