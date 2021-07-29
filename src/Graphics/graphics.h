#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

namespace graphics
{
	class GraphicsRuntime
	{
	public:
		void init();
		void run();
		void cleanup();
	};
}