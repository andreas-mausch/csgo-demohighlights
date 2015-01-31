#include "MemoryBitStream.h"

MemoryBitStream::MemoryBitStream(MemoryStreamBuffer &buffer)
: MemoryStream(buffer), currentBitPosition(0), currentByte(0)
{
}

bool MemoryBitStream::readBit()
{
	if (currentBitPosition == 0)
	{
		read(&currentByte, sizeof(char));
	}

	bool bit = (currentByte >> currentBitPosition) & 0x01;
	currentBitPosition++;
	if (currentBitPosition == 8)
	{
		currentBitPosition = 0;
	}
	return bit;
}

void MemoryBitStream::readBits(void *buffer, int bitCount)
{
	int bitsLeft = bitCount;
	if (bitCount > 8)
	{
		readBytes(buffer, bitCount / 8);
		bitsLeft = bitCount % 8;
	}

	for (int targetBit = bitCount - bitsLeft; targetBit < bitCount; targetBit++)
	{
		reinterpret_cast<char *>(buffer)[targetBit / 8] |= readBit() << (targetBit % 8);
	}
}

char MemoryBitStream::readByte()
{
	char result = 0;

	if (currentBitPosition == 0)
	{
		read(&result, sizeof(char));
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			result |= readBit() << i;
		}
	}

	return result;
}

void MemoryBitStream::readBytes(void *buffer, int length)
{
	if (currentBitPosition == 0)
	{
		read(reinterpret_cast<char *>(buffer), length);
	}
	else
	{
		for (int i = 0; i < length; i++)
		{
			reinterpret_cast<char *>(buffer)[i] = readByte();
		}
	}
}

unsigned int MemoryBitStream::ReadUBitLong(int numbits)
{
	unsigned int result = 0;
	for (int i = 0; i < numbits; i++)
	{
		result |= readBit() << i;
	}
	return result;
}