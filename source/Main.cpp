#include <iostream>
#include <fstream>

#include "sdk/demofile.h"

struct DemoHeader
{
	std::string filestamp;
	int protocol;
	int networkProtocol;
	std::string serverName;
	std::string clientName;
	std::string mapName;
	std::string gameDirectory;
	float playbackTime;
	int playbackTicks;
	int playbackFrames;
	int signonlength;
};

std::string readString(std::istream &stream, int length)
{
	char *buffer = new char[length];
	stream.read(buffer, length);
	std::string result(buffer);
	delete[] buffer;

	return result;
}

unsigned char readByte(std::istream &stream)
{
	unsigned char result;
	stream.read(reinterpret_cast<char *>(&result), sizeof(unsigned char));
	return result;
}

int readInt(std::istream &stream)
{
	int result;
	stream.read(reinterpret_cast<char *>(&result), sizeof(int));
	return result;
}

float readFloat(std::istream &stream)
{
	float result;
	stream.read(reinterpret_cast<char *>(&result), sizeof(float));
	return result;
}

void parseHeader(std::istream &demo)
{
	DemoHeader header;
	header.filestamp = readString(demo, 8);
	header.protocol = readInt(demo);
	header.networkProtocol = readInt(demo);
	header.serverName = readString(demo, MAX_OSPATH);
	header.clientName = readString(demo, MAX_OSPATH);
	header.mapName = readString(demo, MAX_OSPATH);
	header.gameDirectory = readString(demo, MAX_OSPATH);
	header.playbackTime = readFloat(demo);
	header.playbackTicks = readInt(demo);
	header.playbackFrames = readInt(demo);
	header.signonlength = readInt(demo);
}

int main()
{
	std::ifstream demofile("demo.dem");
	std::istream &demo = demofile;

	parseHeader(demo);

	bool end = false;
	while (!end)
	{
		unsigned char command = readByte(demo);
		int tick = readInt(demo);
	}

	return 0;
}
