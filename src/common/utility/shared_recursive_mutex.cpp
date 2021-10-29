#include "shared_recursive_mutex.h"
#include <iostream>

void shared_recursive_mutex::lock()
{
	_m.lock();
	if (_ownerLockCount > 0 && _owner != std::this_thread::get_id())
	{
		_m.unlock();
		while (true)
		{
			_m.lock();
			if (_ownerLockCount == 0)
				break;
			_m.unlock();
			std::this_thread::yield();

		}
	}	
	else
	{
		_m.unlock();
		while (true)
		{
			_m.lock();
			size_t count = 0;
			for (auto c : _sharedOwners)
			{
				if (c.first != std::this_thread::get_id())
					count += c.second;
			}
			if (count == 0)
				break;
			std::this_thread::yield();
			_m.unlock();
		}
	}

	_owner = std::this_thread::get_id();
	_ownerLockCount++;
	//std::cout << "Thread: " << _owner << " locking " << _ownerLockCount << std::endl;
	_m.unlock();
}

void shared_recursive_mutex::unlock()
{
	std::scoped_lock lock(_m);
	_ownerLockCount--;
	//std::cout << "Thread: " << _owner << " unlocking " << _ownerLockCount << std::endl;
	assert(_ownerLockCount >= 0);
}

void shared_recursive_mutex::lock_shared()
{
	_m.lock();
	if (_ownerLockCount > 0 && _owner != std::this_thread::get_id())
	{
		_m.unlock();
		while (true)
		{
			_m.lock();
			if (_ownerLockCount == 0)
				break;
			_m.unlock();
			std::this_thread::yield();
		}
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
