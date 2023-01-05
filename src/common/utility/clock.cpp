#include "clock.h"

std::chrono::high_resolution_clock::time_point Timer::lastFrame;
uint64_t Timer::lastFrameDuration;
double Timer::deltaTime = 0;
unsigned int Timer::fps = 0;

void Timer::startTimer() { lastFrame = std::chrono::high_resolution_clock::now(); }

void Timer::updateTimer()
{
  auto currentFrame = std::chrono::high_resolution_clock::now();
  lastFrameDuration =
      std::chrono::duration<uint64_t, std::chrono::nanoseconds::period>(currentFrame - lastFrame).count();
  deltaTime = lastFrameDuration / 1000000000;
  fps = 1000000000 / lastFrameDuration;
  lastFrame = currentFrame;
}

std::chrono::high_resolution_clock::time_point Stopwatch::_start;
Stopwatch::Stopwatch() { _start = std::chrono::high_resolution_clock::now(); }
