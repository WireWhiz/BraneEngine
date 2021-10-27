#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <assert.h>

class shared_recursive_mutex
{
	std::mutex _m;
	std::atomic<std::thread::id> _owner;
	std::atomic<size_t> _ownerLockCount;
	std::unordered_map<std::thread::id, size_t> _sharedOwners;
public:
	void lock();
	void unlock();
	void lock_shared();
	void unlock_shared();
};