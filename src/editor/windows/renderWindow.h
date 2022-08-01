//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_RENDERWINDOW_H
#define BRANEENGINE_RENDERWINDOW_H

#include <ui/guiWindow.h>
#include "vulkan/vulkan.h"
#include "glm/gtx/quaternion.hpp"
#include "ecs/entityID.h"
#include <vector>
#include <memory>

namespace graphics{
    class RenderTexture;
    class MeshRenderer;
    class SwapChain;
}

class AssetEditorContext;
class RenderWindow : public GUIWindow
{
	graphics::RenderTexture* _texture = nullptr;
	graphics::MeshRenderer* _renderer;
	std::vector<VkDescriptorSet> _imGuiBindings;
	graphics::SwapChain* _swapChain;
	VkExtent2D _windowSize = {0,0};
	bool _queueReload = false;
	uint64_t _frameCount = 0;
	bool _panning = false;
	ImVec2 _lastMousePos;
    bool _manipulating = false;

    std::shared_ptr<AssetEditorContext> _focusedAsset;
    EntityID _focusedEntity;

	float zoom = -5;
	glm::vec3 position = {0,0,0};
	glm::vec2 rotation = {24,-45};
    void displayContent() override;
public:
    RenderWindow(GUI& ui);
    ~RenderWindow();
    void update() override;
	void lookAt(glm::vec3 pos);
	glm::quat rotationQuat() const;
};


#endif //BRANEENGINE_RENDERWINDOW_H
