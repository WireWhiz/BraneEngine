//
// Created by eli on 7/9/2022.
//

#ifndef BRANEENGINE_ASYNCDATA_H
#define BRANEENGINE_ASYNCDATA_H

#include <memory>
#include <functional>
#include <string>
#include <runtime/runtime.h>
#include <utility/threadPool.h>
#include <array>
#include <mutex>

template<typename T>
class AsyncData
{
    struct InternalData{
        std::unique_ptr<T> _data;
        std::function<void(T)> _callback;
        bool _thenMain = false;
        std::string _errorMessage;
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
    AsyncData& thenMain(const std::function<void(T)>& callback)
    {
        std::scoped_lock lock(_instance->_lock);
        _instance->_thenMain = true;
        if(_instance->_data)
        {
            if(IS_MAIN_THREAD())
                callback(std::move(*_instance->_data));
            else
            {
                auto data = std::move(*_instance->_data);
                ThreadPool::enqueueMain([callback, data = std::move(data)]{callback(std::move(data));});
            }
        }
        else
            _instance->_callback = callback;
        return *this;
    }
    AsyncData& onError(std::function<void(const std::string& err)> callback)
    {
        std::scoped_lock lock(_instance->_lock);
        if(_instance->_errorMessage.empty())
            _instance->_error = std::move(callback);
        else
            callback(_instance->_errorMessage);
        return *this;
    }
    void setData(const T& data) const
    {
        std::scoped_lock lock(_instance->_lock);
        if(_instance->_callback)
        {
            if(_instance->_thenMain)
            {
                if(IS_MAIN_THREAD())
                    _instance->_callback(std::move(*_instance->_data));
                else
                {
                    auto callback = std::move(_instance->_callback);
                    ThreadPool::enqueueMain([callback = std::move(callback), data]{callback(data);});
                }
            }
            else
                _instance->_callback(data);
        }
        else
            _instance->_data = std::make_unique<T>(data);
    }
    void setData(T&& data) const
    {
        std::scoped_lock lock(_instance->_lock);
        if(_instance->_callback)
        {
            if(_instance->_thenMain)
            {
                if(IS_MAIN_THREAD())
                    _instance->_callback(std::move(*_instance->_data));
                else
                {
                    auto callback = std::move(_instance->_callback);
                    ThreadPool::enqueueMain([callback = std::move(callback), data = std::move(data)]{callback(std::move(data));});
                }
            }
            else
                _instance->_callback(std::move(data));
        }
        else
            _instance->_data = std::make_unique<T>(std::move(data));
    }
    void setError(const std::string& error) const
    {
        std::scoped_lock lock(_instance->_lock);
        T data;
        if(!_instance->_errorMessage.empty())
            return;
        if(_instance->_error)
        {
            _instance->_errorMessage = error;
            _instance->_error(error);
        }
        else
            _instance->_errorMessage = error;
    }

    [[nodiscard]] bool callbackSet() const
    {
        std::scoped_lock lock(_instance->_lock);
        return (bool)(_instance->_callback);
    }
};

#endif //BRANEENGINE_ASYNCDATA_H
