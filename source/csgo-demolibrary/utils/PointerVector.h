#pragma once

#include <vector>

template <class T>
class PointerVector
{
private:
	std::vector<T *> pointers;

public:
	typedef T** iterator;
	typedef T* const * const_iterator;

	PointerVector() {}
	~PointerVector()
	{
		for (std::vector<T *>::iterator it = pointers.begin(); it != pointers.end(); it++)
		{
			delete *it;
		}
	}

	PointerVector(const PointerVector &other)
	{
		*this = other;
	}

	PointerVector &PointerVector::operator = (const PointerVector &other)
	{
		clear();
		for (PointerVector<T>::const_iterator it = other.begin(); it != other.end(); it++)
		{
			push_back(new T(**it));
		}

		return *this;
	}

	void clear()
	{
		for (std::vector<T *>::iterator it = pointers.begin(); it != pointers.end(); it++)
		{
			delete *it;
		}
		pointers.clear();
	}

	void push_back(T *element)
	{
		pointers.push_back(element);
	}

	int size() const
	{
		return pointers.size();
	}

	void erase(iterator it)
	{
		delete *it;
		int index = it - begin();
		pointers.erase(pointers.begin() + index);
	}

	T& operator [] (int index)
	{
		return *pointers[index];
	}

	iterator begin()
	{
		if (pointers.size() == 0)
		{
			return NULL;
		}

		return &pointers[0];
	}
	const_iterator begin() const
	{
		if (pointers.size() == 0)
		{
			return NULL;
		}

		return &pointers[0];
	}

	iterator end() { return begin() + size(); }
	const_iterator end() const { return begin() + size(); }

};

// #define foreach(PTR_VECTOR, PTR_TYPE) for (int __x = 0; __x < PTR_VECTOR.size(); __x++) { PTR_TYPE = PTR_VECTOR[__x];
