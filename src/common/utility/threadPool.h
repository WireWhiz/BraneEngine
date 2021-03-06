#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <vector>
#include <queue>
#include <memory>
#include <system_error>
#include <cassert>
#include <condition_variable>

#include <iostream>

//Eventually I might want to move this into the runtime class

class JobHandle
{
	std::atomic<size_t> _instances;
	std::function<void()> _next;
	std::shared_ptr<JobHandle> _nextHandle;
	friend class ThreadPool;
	void enqueueNext();
public:
	bool finished();
	void finish();
	std::shared_ptr<JobHandle> next(std::function<void ()> f);
	JobHandle();
};

struct Job
{
	std::function<void()> f;
	std::shared_ptr<JobHandle> handle;
	void operator=(const Job& job);
	Job(std::function<void()> f, std::shared_ptr<JobHandle> handle);
	Job() {};
};

class ThreadPool
{
	static size_t _instances;
	static std::vector<std::thread> _threads;
	static size_t _staticThreads;
	static size_t _minThreads;


	static std::mutex _queueMutex;
	static std::condition_variable _workAvailable;
	static std::queue<Job> _jobs;
	static std::atomic_bool _running;
	static int threadRuntime();


public:
	static std::thread::id main_thread_id;
	static void init(size_t minThreads);
	static void cleanup();
	static std::shared_ptr<JobHandle> addStaticThread(std::function<void()> function);
	static void addStaticTimedThread(std::function<void()> function, std::chrono::seconds interval);
	static std::shared_ptr<JobHandle> enqueue(std::function<void()> function);
	static void enqueue(std::function<void()> function, std::shared_ptr<JobHandle>);
	static std::shared_ptr<JobHandle> enqueueBatch(std::vector<std::function<void()>> functions);
};

#define ASSERT_MAIN_THREAD() assert(std::this_thread::get_id() == ThreadPool::main_thread_id)