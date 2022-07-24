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
	AsyncData& then(const std::function<void(T)>& callback)
	{
		if(_instance->_data)
			callback(std::move(*_instance->_data));
		else
			_instance->_callback = callback;
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
class AsyncAction
{
	std::atomic<size_t> _loaded_count;
	std::vector<AsyncData<T>> _handles;
    std::vector<T> _data;
    std::function<void(std::vector<T>& data)> _onLoaded;
    std::function<void(const std::string& error)> _onFail;
public:
	AsyncAction(std::vector<AsyncData<T>>&& handles, std::function<void(std::vector<T>& data)> onLoaded, std::function<void(const std::string& error)> onFail) :
                _handles(std::move(handles)), _onLoaded(std::move(onLoaded)), _onFail(std::move(onFail))
    {
        _data.resize(_handles.size());
        _loaded_count = 0;
        for (size_t i = 0; i < _handles.size(); ++i)
        {
            _handles[i].then([this, i](T data){
                _data[i] = std::move(data);
                if(++_loaded_count == _data.size())
                    _onLoaded(_data);
            }).onError(_onFail);
        }
    }
};

#endif //BRANEENGINE_ASYNCDATA_H
