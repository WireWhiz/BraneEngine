//
// Created by eli on 5/21/2022.
//

#include "consoleWindow.h"
#include <runtime/logging.h>

ConsoleWindow::ConsoleWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	Logging::addListener([this](const auto& log){
		_messages.push_back(CachedLog{log.toString(), log.level});
	});
}

void ConsoleWindow::draw()
{
	if(ImGui::Begin("Console", &_open, ImGuiWindowFlags_None)){
		ImGui::PushStyleColor(ImGuiCol_ChildBg, {0,0,0,1});
		ImGui::BeginChild("messages");
		const ImVec4 textColors[] = {
				{1,0,0,1},
				{0,1,1,1},
				{1,1,1,1},
				{1,1,1,1}
		};
		ImGui::PushTextWrapPos();
		ImGui::PushStyleColor(ImGuiCol_FrameBg, {0,0,0,1});
		int messageID = 0;
		for(auto& m : _messages)
		{
			ImGui::PushID(messageID++);
			ImGui::PushStyleColor(ImGuiCol_Text, textColors[static_cast<size_t>(m.level)]);
			ImGui::InputTextEx("##m", "", m.text.data(), m.text.size(), {ImGui::GetContentRegionAvail().x, 0}, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
			ImGui::PopID();
		}
		ImGui::PopStyleColor();
		ImGui::PopTextWrapPos();
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	ImGui::End();
}
