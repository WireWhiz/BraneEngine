//
// Created by eli on 5/21/2022.
//

#include "RenderWindow.h"
#include "../editorUI.h"

RenderWindow::RenderWindow(EditorUI& ui) : EditorWindow(ui)
{
	graphics::VulkanRuntime* vkr = ((graphics::VulkanRuntime*)ui.runtime().getModule("graphics"));
	_renderer = vkr->createRenderer();
	_renderer->setRenderCallback([this](auto b){renderScene(b);});
	_renderer->setClearColor({
			                         (float)glm::sin((_frameCount / (double)30.0) * glm::two_pi<double>()),
			                         (float)glm::sin(((_frameCount + 10) / (double)30.0) * glm::two_pi<double>()),
			                         (float)glm::sin(((_frameCount + 20) / (double)30.0) * glm::two_pi<double>()),1});
	_swapChain = vkr->swapChain();
}

RenderWindow::~RenderWindow()
{
	if(_texture)
		delete _texture;
	_texture = nullptr;
}

void RenderWindow::update()
{
	_renderer->setClearColor({
			                         (float)glm::sin((_frameCount / (double)144.0) * glm::two_pi<double>()),
			                         (float)glm::sin(((_frameCount + 48) / (double)144.0) * glm::two_pi<double>()),
			                         (float)glm::sin(((_frameCount + 96) / (double)144.0) * glm::two_pi<double>()),1});
	_frameCount++;

	if(_queueReload)
	{
		vkDeviceWaitIdle(graphics::device->get());
		_renderer->clearTarget();
		if(!_imGuiBindings.empty())
			vkFreeDescriptorSets(graphics::device->get(), _ui.descriptorPool(), _imGuiBindings.size(), _imGuiBindings.data());
		_imGuiBindings.resize(0);
		delete _texture;
		_texture = nullptr;
		_queueReload = false;
	}

	if(!_texture && (_windowSize.width != 0 && _windowSize.height != 0))
	{
		_texture = new graphics::RenderTexture(_windowSize, true, *_swapChain);
		auto& images = _texture->images();
		_imGuiBindings.resize(0);
		for(auto image : images)
		{
			_imGuiBindings.push_back(ImGui_ImplVulkan_AddTexture(_texture->sampler(), image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}
		_renderer->setTarget(_texture);
	}
}

void RenderWindow::draw()
{
	if(ImGui::Begin("Render", nullptr, ImGuiWindowFlags_None)){
		auto window = ImGui::GetContentRegionAvail();
		_windowSize = {static_cast<uint32_t>(glm::floor(glm::max((float)0,window.x))), static_cast<uint32_t>(glm::floor(glm::max((float)0,window.y)))};
		if(_windowSize.width != 0 && _windowSize.height != 0)
		{
			if(_texture && (_windowSize.width != _texture->size().width ||
			                _windowSize.height != _texture->size().height))
			{
				_queueReload = true;
			}
			if(_texture)
				ImGui::Image(_imGuiBindings[_swapChain->currentFrame()], window);
		}
	}
	ImGui::End();
}

void RenderWindow::renderScene(VkCommandBuffer cmdBuffer)
{

}




