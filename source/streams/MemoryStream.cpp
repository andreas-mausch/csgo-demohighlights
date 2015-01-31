#include "MemoryStream.h"
#include "MemoryStreamBuffer.h"

MemoryStream::MemoryStream(MemoryStreamBuffer &buffer)
: std::istream(&buffer)
{
}

MemoryStream::~MemoryStream()
{
}

char MemoryStream::readByte()
{
	char result = 0;
	read(&result, sizeof(char));
	return result;
}

void MemoryStream::readBytes(void *buffer, int length)
{
	read(reinterpret_cast<char *>(buffer), length);
}

std::string MemoryStream::readFixedLengthString(int length)
{
	char *buffer = new char[length];
	readBytes(buffer, length);
	std::string result(buffer);
	delete[] buffer;

	return result;
}

std::string MemoryStream::readNullTerminatedString(int maximumLength)
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

short MemoryStream::readWord()
{
	short result;
	readBytes(&result, sizeof(short));
	return result;
}

int MemoryStream::readInt()
{
	int result;
	readBytes(&result, sizeof(int));
	return result;
}

float MemoryStream::readFloat()
{
	float result;
	readBytes(&result, sizeof(float));
	return result;
}

int MemoryStream::readVarInt32()
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
