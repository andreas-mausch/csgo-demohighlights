#pragma once

template <class T>
class ScopedPointer
{
private:
	T *value;

	ScopedPointer(const ScopedPointer &);
	void operator=(const ScopedPointer &);

public:
	typedef T element_type;

	ScopedPointer(T *value) : value(value) {}
	~ScopedPointer()
	{
		enum { type_must_be_complete = sizeof(T) };
		delete value;
	}

	T& operator *() const
	{
		return *value;
	}
	T& operator ->() const
	{
		return *value;
	}

	T *get() const
	{
		return value;
	}
};
