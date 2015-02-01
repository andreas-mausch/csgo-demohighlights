#pragma once

#include "MemoryStream.h"

class MemoryBitStream : public MemoryStream
{
private:
	int currentBitPosition;
	char currentByte;

public:
	MemoryBitStream(MemoryStreamBuffer &buffer);

	int getCurrentBitPosition();
	char getCurrentByte();

	bool readBit();
	void readBits(void *buffer, int bitCount);
	char readByte();
	void readBytes(void *buffer, int length);
	unsigned int ReadUBitLong(int numbits);
	int ReadSBitLong(int numbits);
	unsigned int ReadUBitVar();

};
