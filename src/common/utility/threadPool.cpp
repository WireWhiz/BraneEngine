#include "threadPool.h"

std::vector<std::thread> ThreadPool::_threads;
std::atomic_bool ThreadPool::_running = true;
std::queue<ThreadPool::Job> ThreadPool::_jobs;
std::mutex ThreadPool::_queueMutex;

int ThreadPool::threadRuntime()
{
	while (_running)
	{
		Job job;

		// Lock queue and retreve job
		_queueMutex.lock();
		bool jobAvalible = !_jobs.empty();
		if (jobAvalible)
		{
			job = _jobs.front();
			_jobs.pop();
		}
		_queueMutex.unlock();

		// if we got a job, run it outside of the lock
		if (jobAvalible)
		{
			try
			{
				job.f();

			}
			catch (const std::exception& e)
			{
				std::cerr << "Thread Error: " << e.what() << std::endl;
			}
			job.handle->_finished = true;
		}

		std::this_thread::yield();
	}
	return 0;
}

void ThreadPool::init()
{
	int threadCount = std::thread::hardware_concurrency();
	_threads.reserve(threadCount);
	for (size_t i = 0; i < threadCount; i++)
	{
		_threads.push_back(std::thread(threadRuntime));
	}
}

void ThreadPool::cleanup()
{
	_running = false;
	for (size_t i = 0; i < _threads.size(); i++)
	{
		_threads[i].join();
	}
}

std::shared_ptr<JobHandle> ThreadPool::enqueue(std::function<void()> function)
{
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();
	_queueMutex.lock();
	_jobs.push(Job( function, handle));
	_queueMutex.unlock();
	return handle;
}

bool JobHandle::finished()
{
	return _finished;
}
void JobHandle::finish()
{
	while (!_finished)
	{
		std::this_thread::yield();
	}
}

JobHandle::JobHandle()
{
	_finished = false;;
}

void ThreadPool::Job::operator=(const Job& job)
{
	f = job.f;
	handle = job.handle;
}

ThreadPool::Job::Job(std::function<void()> f, std::shared_ptr<JobHandle> handle)
{
	this->f = f;
	this->handle = handle;
}
