//
// Created by eli on 8/16/2022.
//

#include "selectProjectWindow.h"
#include "fileManager/fileManager.h"
#include "editor/editor.h"
#include "misc/cpp/imgui_stdlib.h"
#include "tinyfiledialogs.h"
#include "ui/gui.h"

SelectProjectWindow::SelectProjectWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor)
{
	loadRecent();
	_name = "Select Project";
	//TODO store a default project folder in config or grab the documents folder from the OS
	_projectPath = std::filesystem::current_path().append("/Brane Projects/").string();
	_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
}

void SelectProjectWindow::displayContent()
{
	ImVec2 windowSize = {600, 350};
	ImGui::SetWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetWindowPos({(ImGui::GetIO().DisplaySize.x - windowSize.x) / 2, (ImGui::GetIO().DisplaySize.y - windowSize.y) / 2}, ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.3f);
	ImGui::PushFont(_ui.fonts()[1]);
	ImGui::Text("Create New");
	ImGui::PopFont();
	ImGui::InputText("Name", &_projectName, ImGuiInputTextFlags_AutoSelectAll);
	std::string pathStr = _projectPath.string();
	if(ImGui::Button("Select##Directory"))
	{
		char* pathCStr = tinyfd_selectFolderDialog("Select Directory", _projectPath.string().c_str());
		if(pathCStr)
			_projectPath = pathCStr;
	}
	ImGui::SameLine();
	ImGui::InputText("Directory", &pathStr, ImGuiInputTextFlags_AutoSelectAll);
	_projectPath = pathStr;

	if(ImGui::Button("Create"))
	{
		_recentProjects.insert(_recentProjects.begin(), {_projectName, (_projectPath / _projectName / (_projectName + ".brane")).string()});
		_selectedProject = 0;
		saveRecents();
		Runtime::getModule<Editor>()->createProject(_projectName, _projectPath);
	}
	ImGui::Separator();

	ImGui::PushFont(_ui.fonts()[1]);
	ImGui::Text("Recent Projects", ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PopFont();

	ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.1f, 0.1f, 0.1f, 1.0f});
	ImGui::BeginChild("Recent", {ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight() * 0.45f}, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	for(uint8_t i = 0; i < _recentProjects.size(); ++i)
	{
		auto& p = _recentProjects[i];
		if(ImGui::Selectable(p.name.c_str(), _selectedProject == i)){
			_selectedProject = i;
		}
		if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)){
			_selectedProject = i;
			saveRecents();
			Runtime::getModule<Editor>()->loadProject(_recentProjects[i].path);
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::BeginDisabled(_selectedProject < 0);
	if(ImGui::Button("Select##Project")){
		saveRecents();
		Runtime::getModule<Editor>()->loadProject(_recentProjects[_selectedProject].path);
	}
	ImGui::EndDisabled();
	if(ImGui::Button("Open From File"))
	{
		const char* filters[] = {"*.brane"};
		std::string path = tinyfd_openFileDialog("Select Brane File", nullptr, 1, filters, "Brane Project Files", 0);
		_recentProjects.insert(_recentProjects.begin(), {"no name (This is a bug)", path});
		_selectedProject = 0;
		saveRecents();
		Runtime::getModule<Editor>()->loadProject(_recentProjects[_selectedProject].path);
	}
	ImGui::PopStyleVar();
}

void SelectProjectWindow::loadRecent()
{
	try{
		Json::Value projects;
		if(FileManager::readFile("cache/recentProjects.json", projects))
		{
			for(auto& proj : projects["projects"])
			{
				RecentProject p;
				p.name = proj["name"].asString();
				if(p.name.empty())
					p.name = "[No name found]";
				p.path = proj["path"].asString();
				_recentProjects.push_back(p);
			}
			return;
		}
	}catch(...)
	{
		Runtime::log("no usable project cache found, a new one will be created");
	}

}

void SelectProjectWindow::saveRecents()
{
	assert(_selectedProject >= 0);
	if(_selectedProject != 0)
	{
		auto project = _recentProjects[_selectedProject];
		_recentProjects.erase(_recentProjects.begin() + _selectedProject);
		_recentProjects.insert(_recentProjects.begin(), project);
	}
	Json::Value projects;
	for(auto& p : _recentProjects)
	{
		Json::Value proj;
		proj["name"] = p.name;
		proj["path"] = p.path;
		projects["projects"].append(proj);
	}
	FileManager::writeFile("cache/recentProjects.json", projects);
}
