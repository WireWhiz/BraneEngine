//
// Created by eli on 4/26/2022.
//

#include "editorUI.h"
#include <runtime/runtime.h>

EditorUI::EditorUI(Runtime& runtime) : Module(runtime)
{
	runtime.timeline().addBlockBefore("editorUI", "draw");
	runtime.timeline().addTask("drawEditorUI", [&]{
		drawUI();
	}, "editorUI");

	ImGuiID dockspace_id = ImGui::GetID("rootDockSpace");
	ImGui::DockBuilderDockWindow("3D Preview", dockspace_id);
	ImGui::DockBuilderFinish(dockspace_id);
}

const char* EditorUI::name()
{
	return "editorUI";
}

void EditorUI::drawUI()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
	static bool open = true;
	ImGui::Begin("Brane Editor", &open, window_flags);
	ImGui::PopStyleVar(2);

	ImGui::Text("Hello Editor");
	ImGuiID dockspace_id = ImGui::GetID("rootDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	ImGui::End();

	ImGui::Begin("3D Preview");
	ImGui::Text("3D Preview here");
	ImGui::End();


}
