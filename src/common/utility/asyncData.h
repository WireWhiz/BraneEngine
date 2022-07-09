//
// Created by eli on 7/9/2022.
//

#ifndef BRANEENGINE_ASYNCDATA_H
#define BRANEENGINE_ASYNCDATA_H

#include <memory>
#include <functional>
#include <string>
#include <runtime/runtime.h>

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
			Runtime::error(error);
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

#endif //BRANEENGINE_ASYNCDATA_H
