#pragma once
#include <chrono>
#include <thread>

class Timer
{
public:
	static std::chrono::high_resolution_clock::time_point lastFrame;
	static std::chrono::nanoseconds lastFrameDuration;
	static double deltaTime;
	static unsigned int fps;

	static void startTimer()
	{
		lastFrame = std::chrono::high_resolution_clock::now();
	}
	static void updateTimer()
	{
		auto currentFrame = std::chrono::high_resolution_clock::now();
		lastFrameDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(currentFrame - lastFrame);
		deltaTime = static_cast<double>(lastFrameDuration.count()) / static_cast<double>(1000000000);
		fps = static_cast<unsigned int>(1000000000 / lastFrameDuration.count());
		lastFrame = currentFrame;
	}
};
std::chrono::high_resolution_clock::time_point Timer::lastFrame;
std::chrono::nanoseconds Timer::lastFrameDuration;
double Timer::deltaTime = 0;
unsigned int Timer::fps = 0;