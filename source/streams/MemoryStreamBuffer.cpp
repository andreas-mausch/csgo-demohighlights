#include "MemoryStreamBuffer.h"

MemoryStreamBuffer::MemoryStreamBuffer(const char *buffer, size_t length)
{
	char *p = const_cast<char *>(buffer);
	setg(p, p, p + length);
}

std::ios::pos_type MemoryStreamBuffer::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which)
{
	int newPosition = off;

	if (dir == std::ios_base::cur)
	{
		newPosition += gptr() - eback();
	}

	if (dir == std::ios_base::end)
	{
		newPosition += egptr() - eback();
	}

	setg(eback(), eback() + newPosition, egptr());
	return newPosition;
}

std::ios::pos_type MemoryStreamBuffer::seekpos(pos_type pos, std::ios_base::openmode which)
{
	setg(eback(), eback() + pos, egptr());
	return pos;
}
