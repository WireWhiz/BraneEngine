#include "BraneEngine.h"
#include "core/VirtualSystemManager.h"
#include <atomic>
#include <thread>

void printFps(std::atomic<bool>* printing)
{
	while(*printing)
	{
		std::cout << "Fps: " << Timer::fps << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));   
	}
}

int main()
{
	//Run all our tests if we are in debug mode
#ifdef DEBUG
	tests::runTests();
#endif // !NDEBUG

	graphics::GraphicsRuntime gr;
	Timer::startTimer();
	std::atomic<bool> printing = true;
	std::thread fpsCounter(printFps, &printing);

	while (!gr.windowClosed())
	{
		gr.update();
		Timer::updateTimer();
		
	}
	printing = false;
	fpsCounter.join();
#if defined( DEBUG) && defined(__linux__)
	int input;
	std::cin >> input;
#endif
	return 0;
}
