#pragma once

#include "config/config.h"
#include "runtime/module.h"

class AssetManager;
namespace graphics
{
	class VulkanRuntime;
}


class Client : public Module
{
	void addAssetPreprocessors(AssetManager& am, graphics::VulkanRuntime& vkr);

public:
	Client(Runtime& rt);

	const char * name() override;
};