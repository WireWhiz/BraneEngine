//
// Created by eli on 3/7/2022.
//

#ifndef BRANEENGINE_STATICINDEXVECTOR_H
#define BRANEENGINE_STATICINDEXVECTOR_H

#include <vector>
#include <cassert>
#include <stack>
#include <functional>

template<typename T>
class staticIndexVector
{
	std::vector<T> _data;
	std::vector<bool> _isUsed;
	std::stack<size_t> _unused;
	size_t _size = 0;

public:
	size_t push(const T& element)
	{
		size_t index;
		if(!_unused.empty())
		{
			index = _unused.top();
			_data[index] = element;
			_isUsed[index] = true;
			_unused.pop();
		}
		else
		{
			index = _data.size();
			_data.push_back(element);
			_isUsed.push_back(true);
		}
		_size++;
		return index;
	}

	size_t push(T&& element)
	{
		size_t index;
		if(!_unused.empty())
		{
			index = _unused.top();
			_data[index] = std::move(element);
			_isUsed[index] = true;
			_unused.pop();
		}
		else
		{
			index = _data.size();
			_data.push_back(std::move(element));
			_isUsed.push_back(true);
		}
		_size++;
		return index;
	}

	void remove(size_t index)
	{
		_unused.push(index);
		_isUsed[index] = false;
		_size--;
	}

	size_t size() const
	{
		return _size;
	}

	void clear()
	{
		_size = 0;
		_isUsed.clear();
		while(!_unused.empty())
			_unused.pop();
		_data.clear();
	}

	const T& operator [](size_t index) const
	{
		assert(_isUsed[index]);
		return _data[index];
	}

	T& operator [](size_t index)
	{
		assert(_isUsed[index]);
		return _data[index];
	}

	void forEach(const std::function<void(T&)> f)
	{
		for (size_t i = 0; i < _data.size(); ++i)
		{
			if(_isUsed[i])
				f(_data[i]);
		}
	}

	void forEach(const std::function<void(const T&)> f) const
	{
		for (size_t i = 0; i < _data.size(); ++i)
		{
			if(_isUsed[i])
				f(_data[i]);
		}
	}
};

#endif //BRANEENGINE_STATICINDEXVECTOR_H