#pragma once

#include <iostream>

class MemoryStreamBuffer;

class MemoryStream : public std::istream
{
public:
	MemoryStream(MemoryStreamBuffer &buffer);

	char readByte();
	short readWord();
	int readInt();
	float readFloat();
	int readVarInt32();

	void readBytes(void *buffer, int length);
	std::string readFixedLengthString(int length);
	std::string readNullTerminatedString(int maximumLength);
};
