#pragma once

#include "MemoryStream.h"

class MemoryBitStream
{
private:
	int position;

	const char *buffer;
	int length;

public:
	MemoryBitStream(const char *buffer, size_t length);

	int getPosition();

	bool readBit();
	void readBits(void *buffer, int bitCount);
	char readByte();
	void readBytes(void *buffer, int length);
	unsigned int ReadUBitLong(int numbits);
	int ReadSBitLong(int numbits);
	unsigned int ReadUBitVar();

	virtual short readWord();
	virtual int readInt();
	virtual float readFloat();
	virtual int readVarInt32();

	std::string readFixedLengthString(int length);
	std::string readNullTerminatedString(int maximumLength);

};
