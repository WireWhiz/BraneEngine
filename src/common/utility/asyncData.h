//
// Created by eli on 7/9/2022.
//

#ifndef BRANEENGINE_ASYNCDATA_H
#define BRANEENGINE_ASYNCDATA_H

#include <memory>
#include <functional>
#include <string>
#include <runtime/runtime.h>
#include <array>
#include <mutex>

template<typename T>
class AsyncData
{
	struct InternalData{
		std::unique_ptr<T> _data;
		std::function<void(T)> _callback;
		std::function<void(const std::string&)> _error;
		std::mutex _lock;
	};
	std::shared_ptr<InternalData> _instance;
public:
	AsyncData()
	{
		_instance = std::make_shared<InternalData>();
	}
	AsyncData& then(const std::function<void(T)>& callback)
	{
		std::scoped_lock lock(_instance->_lock);
		if(_instance->_data)
			callback(std::move(*_instance->_data));
		else
			_instance->_callback = callback;
		return *this;
	}
	AsyncData& onError(std::function<void(const std::string& err)> callback)
	{
		std::scoped_lock lock(_instance->_lock);
		_instance->_error = std::move(callback);
		return *this;
	}
	void setData(T& data) const
	{
		std::scoped_lock lock(_instance->_lock);
		if(_instance->_callback)
			_instance->_callback(std::move(data));
		else
			_instance->_data = std::make_unique<T>(std::move(data));
	}
	void setData(T&& data) const
	{
		std::scoped_lock lock(_instance->_lock);
		if(_instance->_callback)
			_instance->_callback(std::move(data));
		else
			_instance->_data = std::make_unique<T>(std::move(data));
	}
	void setError(const std::string& error) const
	{
		std::scoped_lock lock(_instance->_lock);
		T data;
		if(_instance->_error)
			_instance->_error(error);
		else
			Runtime::error(error);
	}

	[[nodiscard]] bool callbackSet() const
	{
		std::scoped_lock lock(_instance->_lock);
		return (bool)(_instance->_callback);
	}
};

#endif //BRANEENGINE_ASYNCDATA_H
