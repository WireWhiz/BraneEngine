//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_RENDERWINDOW_H
#define BRANEENGINE_RENDERWINDOW_H

#include "../editorWindow.h"
#include "graphics/RenderTarget.h"
#include "graphics/MeshRenderer.h"
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

class RenderWindow : public EditorWindow
{
	graphics::RenderTexture* _texture = nullptr;
	graphics::MeshRenderer* _renderer;
	std::vector<VkDescriptorSet> _imGuiBindings;
	graphics::SwapChain* _swapChain;
	VkExtent2D _windowSize = {0,0};
	bool _queueReload = false;
	uint64_t _frameCount = 0;

	float zoom = 0;
	glm::vec3 position = {0,0,0};
	glm::vec2 rotation = {0,0};
public:
	RenderWindow(EditorUI& ui);
	~RenderWindow();
	void update() override;
	void draw() override;
};


#endif //BRANEENGINE_RENDERWINDOW_H
