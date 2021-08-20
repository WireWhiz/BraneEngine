#pragma once
#include <chrono>
#include <thread>

class Timer
{
public:
	static std::chrono::high_resolution_clock::time_point lastFrame;
	static uint64_t lastFrameDuration;
	static double deltaTime;
	static unsigned int fps;

	static void startTimer();
	static void updateTimer();
};

class Stopwatch
{
	static std::chrono::high_resolution_clock::time_point _start;
public:
	Stopwatch();
	long long time();
};