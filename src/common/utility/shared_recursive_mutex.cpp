#include "shared_recursive_mutex.h"
#include <iostream>


size_t shared_recursive_mutex::srmIdCounter = 0;
shared_recursive_mutex::shared_recursive_mutex()
{
	srmId = srmIdCounter++;
}

void shared_recursive_mutex::lock()
{
	while (true)
	{
		_m.lock();
		if (_sharedOwners == 0 && (_ownerLockCount == 0 || _owner == std::this_thread::get_id()))
			break;
		_m.unlock();
		std::this_thread::yield();

	}

	_owner = std::this_thread::get_id();
	_ownerLockCount += 1;
	_m.unlock();
}

void shared_recursive_mutex::unlock()
{
	std::scoped_lock lock(_m);
	assert(_ownerLockCount != 0);
	_ownerLockCount -= 1;
}

void shared_recursive_mutex::lock_shared()
{
	while (true)
	{
		_m.lock();
		if (_ownerLockCount == 0 || _owner == std::this_thread::get_id())
			break;
		_m.unlock();
		std::this_thread::yield();
	}
	_sharedOwners += 1;
	_m.unlock();
}

void shared_recursive_mutex::unlock_shared()
{
	std::scoped_lock lock(_m);
	assert(_sharedOwners != 0);
	_sharedOwners -= 1;

}
