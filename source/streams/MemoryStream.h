#pragma once

#include <iostream>

class MemoryStreamBuffer;

class MemoryStream : public std::istream
{
public:
	MemoryStream(MemoryStreamBuffer &buffer);
	virtual ~MemoryStream();

	virtual char readByte();
	virtual short readWord();
	virtual int readInt();
	virtual float readFloat();
	virtual int readVarInt32();

	virtual void readBytes(void *buffer, int length);
	std::string readFixedLengthString(int length);
	std::string readNullTerminatedString(int maximumLength);
};
