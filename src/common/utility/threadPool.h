#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <vector>
#include <queue>
#include <system_error>

#include <iostream>

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
	static std::queue<Job> _jobs;
	static std::atomic_bool _running;
	static int threadRuntime();


public:
	static std::thread::id main_thread_id;
	static void init(size_t minThreads);
	static void cleanup();
	static void addStaticThread(std::function<void()> function);
	static void addStaticTimedThread(std::function<void()> function, std::chrono::seconds interval);
	static std::shared_ptr<JobHandle> enqueue(std::function<void()> function);
	static void enqueue(std::function<void()> function, std::shared_ptr<JobHandle>);
	static std::shared_ptr<JobHandle> enqueueBatch(std::vector<std::function<void()>> functions);
};

template<typename T>
class AsyncData
{
	std::function<void(T data, bool success, const std::string& error)> _callback;
public:
	void callback(std::function<void(T data, bool success, const std::string& error)> callback)
	{
		_callback = callback;
	}
	void setData(T data) const
	{
		_callback(data, true, "");
	}
	void setError(const std::string& error) const
	{
		T data;
		_callback(data, false, error);
	}

	[[nodiscard]] bool callbackSet() const
	{
		return (bool)_callback;
	}
};

template<typename T>
class AsyncDataArray
{
	size_t _size;
	std::atomic<size_t> _loaded_count;
	std::function<void(bool success, const std::string& error)> _fullyLoaded;
	std::function<void(size_t index, T data)> _indexLoaded;
public:
	explicit AsyncDataArray(size_t size)
	{
		_size = size;
		_loaded_count = 0;
	}
	void fullyLoaded(std::function<void(bool success, const std::string& error)> callback)
	{
		_fullyLoaded = std::move(callback);
	}
	void indexLoaded(std::function<void(size_t index, T data)> callback)
	{
		_indexLoaded = std::move(callback);
	}
	void setData(size_t index, T data)
	{
		assert(index < _size);
		assert(_loaded_count < _size);
		assert(_indexLoaded);
		assert(_fullyLoaded);

		_indexLoaded(index, data);
		_loaded_count++;
		if(_loaded_count == _size && _fullyLoaded)
			_fullyLoaded(true, "");

	}
	void setError(const std::string& error)
	{
		if(_fullyLoaded)
			_fullyLoaded(false, error);
	}

	AsyncData<T> operator[](size_t index)
	{
		AsyncData<T> o;
		o.callback([this, index](T data, bool successful, const std::string& error){
			if(successful)
				setData(index, data);
			else
				setError(error);
		});
		return o;
	}
};

#define ASSERT_MAIN_THREAD() assert(std::this_thread::get_id() == ThreadPool::main_thread_id)