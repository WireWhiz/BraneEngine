#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <assert.h>

class shared_recursive_mutex
{

	static size_t srmIdCounter;
	size_t srmId;
	std::mutex _m;
	std::thread::id _owner;
	size_t _ownerLockCount = 0;
	size_t _sharedOwners = 0;
public:
	shared_recursive_mutex();
	void lock();
	void unlock();
	void lock_shared();
	void unlock_shared();
};