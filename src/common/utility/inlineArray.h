//
// Created by eli on 6/29/2022.
//

#ifndef BRANEENGINE_INLINEARRAY_H
#define BRANEENGINE_INLINEARRAY_H

#include <cstdint>
#include <cassert>
#include <vector>

template<typename T, size_t Count>
class InlineArray
{
	T _localData[Count];
	std::vector<T>* _externalData = nullptr;
	size_t _size = 0;
public:
	InlineArray() = default;
	InlineArray(const InlineArray& o) noexcept
	{
		for (size_t i = 0; i < Count; ++i)
		{
			_localData[i] = o._localData;
		}
		_size = o._size;
		_externalData = nullptr;
		if(o._externalData)
		{
			_externalData = new std::vector<T>(o._externalData->size());
			std::copy(o._externalData->begin(), o._externalData->end(), _externalData->begin());
		}

	}
	InlineArray& operator==(InlineArray&& o) noexcept
	{
		for (size_t i = 0; i < Count; ++i)
		{
			_localData[i] = std::move(o._localData);
		}
		_externalData = o._externalData;
		_size = o._size;
		o._externalData = nullptr;
	}
	~InlineArray()
	{
		if(_externalData)
			delete _externalData;
	}
	T& operator[](size_t index)
	{
		assert(index < _size);
		if(index < Count)
			return _localData[index];

		assert(_externalData);
		index -= Count;
		return (*_externalData)[index];
	}

	void push_back(const T& item)
	{
		if(_size < Count)
		{
			_localData[_size++] = item;
			return;
		}

		if(!_externalData)
			_externalData = new std::vector<T>();
		_externalData->push_back(item);
		++_size;
	}

	void push_back(T&& item)
	{
		if(_size < Count)
		{
			_localData[_size++] = item;
			return;
		}

		if(!_externalData)
			_externalData = new std::vector<T>();
		_externalData->push_back(item);
		++_size;
	}

	class iterator{
		InlineArray<T, Count>& _ref;
		size_t _index;
	public:
		iterator(InlineArray<T, Count>& ref, size_t index) : _ref(ref), _index(index){};
		void operator++()
		{
			++_index;
		}
		void operator+(size_t index)
		{
			_index += index;
		}
		bool operator!=(const iterator& o) const
		{
			return _index != o.index;
		}
		bool operator==(const iterator& o) const
		{
			return _index == o.index;
		}
		T& operator*()
		{
			return _ref[_index];
		};

		using iterator_category = std::random_access_iterator_tag;
		using reference = T&;
		using pointer = T*;
	};
	iterator begin()
	{
		return iterator(*this, 0);
	}
	iterator end()
	{
		return iterator(*this, _size);
	}
};

typedef InlineArray<uint32_t, 4> inlineUIntArray;
typedef InlineArray<int32_t, 4> inlineIntArray;
typedef InlineArray<float, 4> inlineFloatArray;


#endif //BRANEENGINE_INLINEARRAY_H
