#include <iostream>
#include <fstream>

#include "sdk/demofile.h"

#include <stdarg.h>  // For va_start, etc.

std::string string_format(const std::string fmt, ...) {
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}

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

void parsePacket(std::istream &demo)
{
	std::cout << "Parse packet" << std::endl;
	democmdinfo_t cmdinfo;
	demo.read(reinterpret_cast<char *>(&cmdinfo), sizeof(democmdinfo_t));
	int sequenceNumberIn = readInt(demo);
	int sequenceNumberOut = readInt(demo);
	int length = readInt(demo);
	std::cout << "Parse packet length: " << length << std::endl;
	demo.seekg(length, std::ios_base::cur);
	std::cout << "Parse packet end" << std::endl;
}

void unhandledCommand(const std::string &description)
{
	std::cout << "Unhandled command: " << description << std::endl;
	exit(1);
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
		unsigned char playerSlot = readByte(demo);
		std::cout << "command: " << ((int)command) << std::endl;

		switch (command)
		{
		case dem_signon:
		case dem_packet:
			parsePacket(demo);
			break;
		case dem_synctick:
			break;
		case dem_consolecmd:
			break;
		case dem_usercmd:
			break;
		case dem_datatables:
			break;
		case dem_stop:
			end = true;
			break;
		case dem_customdata:
			break;
		case dem_stringtables:
			break;
		default:
			unhandledCommand(string_format("command: default %d", command));
		}
	}

	return 0;
}
