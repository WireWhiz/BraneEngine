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

template<typename T>
class AsyncData
{
	struct InternalData{
		std::unique_ptr<T> _data;
		std::function<void(T)> _callback;
		std::function<void(const std::string&)> _error;
	};
	std::shared_ptr<InternalData> _instance;
public:
	AsyncData()
	{
		_instance = std::make_shared<InternalData>();
	}
	AsyncData& then(std::function<void(T)> callback)
	{
		if(_instance->_data)
			callback(std::move(*_instance->_data));
		else
			_instance->_callback = std::move(callback);
		return *this;
	}
	AsyncData& onError(std::function<void(const std::string& err)> callback)
	{
		_instance->_error = std::move(callback);
		return *this;
	}
	void setData(T& data) const
	{
		if(callbackSet())
			_instance->_callback(std::move(data));
		else
			_instance->_data = std::make_unique<T>(std::move(data));
	}
	void setData(T&& data) const
	{
		if(callbackSet())
			_instance->_callback(std::move(data));
		else
			_instance->_data = std::make_unique<T>(std::move(data));
	}
	void setError(const std::string& error) const
	{
		T data;
		if(_instance->_error)
			_instance->_error(error);
		else
			throw std::runtime_error(error);
	}

	[[nodiscard]] bool callbackSet() const
	{
		return (bool)(_instance->_callback);
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