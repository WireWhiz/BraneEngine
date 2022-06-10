//
// Created by eli on 4/26/2022.
//

#ifndef BRANEENGINE_EDITORUI_H
#define BRANEENGINE_EDITORUI_H

#include <runtime/module.h>
#include <networking/connection.h>
#include "editorWindow.h"
#include "EditorEvent.h"
#include "graphics/graphics.h"
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <deque>
#include <unordered_map>
#include <functional>
#include <graphics/renderer.h>


class EditorUI : public Module
{
	bool _redockQueued;
	graphics::CustomRenderer* _renderer;
	VkDescriptorPool _imGuiDescriptorPool;

	std::vector<std::unique_ptr<EditorWindow>> _windows;
	void drawUI();
	void mainMenu();
	void defaultDocking();

	std::mutex _queueLock;
	std::deque<std::unique_ptr<EditorEvent>> _queuedEvents;
	std::unordered_map<std::string, std::vector<std::function<void(EditorEvent*)>>> _eventListeners;

	net::Connection* _server;

	void callEvents();
	void addMainWindows();
public:
	EditorUI(Runtime& runtime);
	~EditorUI();

	const char* name() override;
	void stop() override;

	Runtime& runtime();
	net::Connection* server() const;

	void removeWindow(EditorWindow* window);

	void sendEvent(std::unique_ptr<EditorEvent>&& name);
	template<typename T>
	void addEventListener(const std::string& name, std::function<void(T*)> callback)
	{
		static_assert(std::is_base_of<EditorEvent, T>());
		if constexpr(std::is_same<EditorEvent, T>())
			_eventListeners[name].push_back(callback);
		else
			_eventListeners[name].push_back([callback](EditorEvent* event){
				T* e = (T*)(event);
				callback(e);
			});
	}

	VkDescriptorPool descriptorPool();
	void setupImGui(graphics::VulkanRuntime& runtime);
	void cleanupImGui();
};


#endif //BRANEENGINE_EDITORUI_H
