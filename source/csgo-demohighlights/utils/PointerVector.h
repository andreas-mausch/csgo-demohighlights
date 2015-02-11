#pragma once

#include <vector>

template <class T>
class PointerVector
{
private:
	std::vector<T *> pointers;

public:
	typedef T** iterator;
	typedef const T** const_iterator;

	PointerVector() {}
	~PointerVector()
	{
		for (std::vector<T *>::iterator it = pointers.begin(); it != pointers.end(); it++)
		{
			delete *it;
		}
	}

	void push_back(T *element)
	{
		pointers.push_back(element);
	}

	int size()
	{
		return pointers.size();
	}

	T& operator [] (int index)
	{
		return *pointers[index];
	}

	iterator begin() { return &pointers[0]; }
	const_iterator begin() const { return &pointers[0]; }
	iterator end() { return begin() + size(); }
	const_iterator end() const { return begin() + size(); }

};

// #define foreach(PTR_VECTOR, PTR_TYPE) for (int __x = 0; __x < PTR_VECTOR.size(); __x++) { PTR_TYPE = PTR_VECTOR[__x];
