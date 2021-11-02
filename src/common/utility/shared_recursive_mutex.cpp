#include "shared_recursive_mutex.h"
#include <iostream>


size_t shared_recursive_mutex::srmIdCounter = 0;
shared_recursive_mutex::shared_recursive_mutex()
{
	srmId = srmIdCounter++;
	_ownerLockCount = 0;
}

void shared_recursive_mutex::lock()
{
	while (true)
	{
		_m.lock();
		size_t count = 0;
		for (auto c : _sharedOwners)
		{
			if (c.first != std::this_thread::get_id())
				count += c.second;
		}
		if (count == 0 && (_ownerLockCount == 0 || _owner == std::this_thread::get_id()))
			break;
		_m.unlock();
		std::this_thread::yield();

	}

	_owner = std::this_thread::get_id();
	_ownerLockCount += 1;
	//std::cout << "Thread: " << _owner << " locking mutex: " << srmId << " (" << _ownerLockCount << ") times." << std::endl;
	_m.unlock();
}

void shared_recursive_mutex::unlock()
{
	std::scoped_lock lock(_m);
	//std::cout << "Thread: " << _owner << " unlocking mutex: " << srmId << " (" << _ownerLockCount << ") times." << std::endl;
	_ownerLockCount -= 1;
	assert(_ownerLockCount != size_t(-1));
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
	
	
	_sharedOwners[std::this_thread::get_id()] += 1;
	//std::cout << "Thread: " << std::this_thread::get_id() << " shared locking " << _sharedOwners[std::this_thread::get_id()] << std::endl;

	_m.unlock();
}

void shared_recursive_mutex::unlock_shared()
{
	std::scoped_lock lock(_m);
	_sharedOwners[std::this_thread::get_id()] -= 1;
	//std::cout << "Thread: " << std::this_thread::get_id() << " shared unlocking " << _sharedOwners[std::this_thread::get_id()] << std::endl;

	assert(_sharedOwners[std::this_thread::get_id()] >= 0);
}
