//
// Created by eli on 7/9/2022.
//

#include "guiRenderer.h"
#include <runtime/runtime.h>
#include "gui.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

void GUIRenderer::render(VkCommandBuffer cmdBuffer)
{
	startRenderPass(cmdBuffer);
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	_gui->drawUI();
	ImGui_ImplVulkan_NewFrame();
	ImGui::Render();
	ImDrawData* ImGuiDrawData = ImGui::GetDrawData();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	ImGui_ImplVulkan_RenderDrawData(ImGuiDrawData, cmdBuffer);
	endRenderPass(cmdBuffer);
}

GUIRenderer::GUIRenderer(graphics::SwapChain& swapChain, GUI* gui) : graphics::Renderer(swapChain)
{
	_gui = gui;
}

void GUIRenderer::rebuild()
{
    Renderer::rebuild();
}
