#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <vector>
#include <queue>
#include <future>

#include <iostream>

class JobHandle
{
	std::atomic<size_t> _instances;
	friend class ThreadPool;
	
public:
	bool finished();
	void finish();
	JobHandle();
};

class ThreadPool
{
	static size_t _instances;
	static std::vector<std::thread> _threads;
	struct Job
	{
		std::function<void()> f;
		std::shared_ptr<JobHandle> handle;
		void operator=(const Job& job);
		Job(std::function<void()> f, std::shared_ptr<JobHandle> handle);
		Job() {};
	};
	static std::mutex _queueMutex;
	static std::queue<Job> _jobs;
	static std::atomic_bool _running;
	static void threadRuntime();

public:
	static void init();
	static void cleanup();
	static std::shared_ptr<JobHandle> enqueue(std::function<void()> function);
	static void enqueue(std::function<void()> function, std::shared_ptr<JobHandle> handle);
};
