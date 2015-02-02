#include "MemoryBitStream.h"

MemoryBitStream::MemoryBitStream(const char *buffer, size_t length)
: position(0), buffer(buffer), length(length)
{
}

int MemoryBitStream::getPosition()
{
	return position;
}

bool MemoryBitStream::readBit()
{
	if (position >= length * 8)
	{
		return false;
	}

	char byte = buffer[position / 8];
	int shift = position % 8;
	bool bit =  (byte >> shift) & 1;
	position++;
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
	if (position % 8 == 0)
	{
		if (position + 8 > length * 8)
		{
			return 0;
		}

		char result = buffer[position / 8];
		position += 8;
		return result;
	}
	else
	{
		char result = 0;
		readBits(&result, 8);
		return result;
	}
}

void MemoryBitStream::readBytes(void *buffer, int length)
{
	if (position % 8 == 0)
	{
		if (position + length * 8 > this->length * 8)
		{
			return;
		}

		memcpy(buffer, &this->buffer[position / 8], length);
		position += length * 8;
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

std::string MemoryBitStream::readFixedLengthString(int length)
{
	char *buffer = new char[length];
	readBytes(buffer, length);
	std::string result(buffer);
	delete[] buffer;

	return result;
}

std::string MemoryBitStream::readNullTerminatedString(int maximumLength)
{
	char *buffer = new char[maximumLength];
	memset(buffer, 0, maximumLength);

	int count = 0;
	while (true)
	{
		char byte = readByte();
		if (byte == 0)
		{
			break;
		}

		if (count >= (maximumLength - 1))
		{
			throw std::out_of_range("maximumLength");
		}

		buffer[count] = byte;
		count++;
	}

	std::string result(buffer);
	delete[] buffer;
	return result;
}

short MemoryBitStream::readWord()
{
	short result;
	readBytes(&result, sizeof(short));
	return result;
}

int MemoryBitStream::readInt()
{
	int result;
	readBytes(&result, sizeof(int));
	return result;
}

float MemoryBitStream::readFloat()
{
	float result;
	readBytes(&result, sizeof(float));
	return result;
}

int MemoryBitStream::readVarInt32()
{
	int maximumBytes = sizeof(int);
	int result = 0;
	unsigned char byte = 0;
	int currentByte = 0;

	do
	{
		if (currentByte + 1 == maximumBytes)
		{
			return result;
		}

		byte = readByte();

		result |= (byte & 0x7F) << (7 * currentByte);
		currentByte++;
	} while (byte & 0x80);

	return result;
}
