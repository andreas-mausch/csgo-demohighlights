#pragma once

#include <iostream>

class MemoryStreamBuffer : public std::basic_streambuf<char>
{
public:
	MemoryStreamBuffer(const char *buffer, size_t length);

	std::ios::pos_type seekoff(off_type off, std::ios_base::seekdir dir,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

	std::ios::pos_type seekpos( pos_type pos,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

};
