#include "MemoryBitStream.h"

MemoryBitStream::MemoryBitStream(MemoryStreamBuffer &buffer)
: MemoryStream(buffer), currentBitPosition(0), currentByte(0)
{
}

int MemoryBitStream::getCurrentBitPosition()
{
	return currentBitPosition;
}

char MemoryBitStream::getCurrentByte()
{
	return currentByte;
}

bool MemoryBitStream::readBit()
{
	if (currentBitPosition == 0)
	{
		currentByte = get();
		//read(&currentByte, 1);
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
		readBits(&result, 8);
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

int MemoryBitStream::ReadSBitLong(int numbits)
{
	int result = ReadUBitLong( numbits );
	return (result << (32 - numbits)) >> (32 - numbits);
}

unsigned int MemoryBitStream::ReadUBitVar()
{
	unsigned int result = ReadUBitLong(6);
	switch(result & (16 | 32))
	{
	case 16:
		result = (result & 15) | (ReadUBitLong(4) << 4);
		break;
	case 32:
		result = (result & 15) | (ReadUBitLong(8) << 4);
		break;
	case 48:
		result = (result & 15) | (ReadUBitLong(32 - 4) << 4);
		break;
	}
	return result;
}
