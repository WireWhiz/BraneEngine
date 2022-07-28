//
// Created by eli on 4/26/2022.
//

#ifndef BRANEENGINE_GUI_H
#define BRANEENGINE_GUI_H

#include "common/runtime/module.h"
#include "common/networking/connection.h"
#include "guiWindow.h"
#include "guiEvent.h"
#include "graphics/graphics.h"
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <deque>
#include <unordered_map>
#include <functional>
#include "guiRenderer.h"
#include "guiPopup.h"

class GUI : public Module
{
	GUIRenderer* _renderer;
	friend class GUIRenderer;
	VkDescriptorPool _imGuiDescriptorPool;

    std::vector<ImFont*> _fonts;

	std::vector<std::unique_ptr<GUIWindow>> _windows;
    std::unique_ptr<GUIPopup> _popup;
    void drawUI();
    std::function<void()> _drawMenu;
	std::mutex _queueLock;

    std::deque<std::unique_ptr<GUIEvent>> _queuedEvents;
    std::unordered_map<std::string, std::vector<std::function<void(const GUIEvent*)>>> _eventListeners;

	void callEvents();
public:
	GUI();
	~GUI();

	static const char* name();
    void start() override;
	void stop() override;

	template<typename WindowT, typename... Args>
	WindowT* addWindow(Args... args)
	{
		GUIWindowID id = _windows.size();
		_windows.push_back(std::make_unique<WindowT>(*this, id, args...));
		return static_cast<WindowT*>(_windows[id].get());
	}
	void setMainMenuCallback(std::function<void()> drawMenu);

    void openPopup(std::unique_ptr<GUIPopup>&& popup);
    void closePopup();
    void sendEvent(std::unique_ptr<GUIEvent>&& name);
    template<typename T>
	void addEventListener(const std::string& name, std::function<void(const T*)> callback)
	{
		static_assert(std::is_base_of<GUIEvent, T>());
		if constexpr(std::is_same<GUIEvent, T>())
			_eventListeners[name].push_back(callback);
		else
			_eventListeners[name].push_back([callback](const GUIEvent* event){
				const T* e = (const T*)(event);
				callback(e);
			});
	}

	VkDescriptorPool descriptorPool();
	void setupImGui(graphics::VulkanRuntime& runtime);
	void cleanupImGui();

    const std::vector<ImFont*>& fonts() const;
};


#endif //BRANEENGINE_GUI_H
