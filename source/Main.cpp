#include <iostream>
#include <fstream>

#include "sdk/demofile.h"
#include "sdk/demofiledump.h"

#include "protobuf/generated/cstrike15_usermessages_public.pb.h"
#include "protobuf/generated/netmessages_public.pb.h"

#include "streams/MemoryStream.h"
#include "streams/MemoryBitStream.h"
#include "streams/MemoryStreamBuffer.h"

#include "utils/EndianConverter.h"
#include "utils/StringFormat.h"

#include "DemoParser.h"
#include "GameState.h"

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

void parseHeader(MemoryStream &demo)
{
	DemoHeader header;
	header.filestamp = demo.readFixedLengthString(8);
	header.protocol = demo.readInt();
	header.networkProtocol = demo.readInt();
	header.serverName = demo.readFixedLengthString(MAX_OSPATH);
	header.clientName = demo.readFixedLengthString(MAX_OSPATH);
	header.mapName = demo.readFixedLengthString(MAX_OSPATH);
	header.gameDirectory = demo.readFixedLengthString(MAX_OSPATH);
	header.playbackTime = demo.readFloat();
	header.playbackTicks = demo.readInt();
	header.playbackFrames = demo.readInt();
	header.signonlength = demo.readInt();
}

int main()
{
	std::ifstream demofile("MLGXGA2015-GF-ldlc-vs-nip-dust2.dem", std::ios_base::binary);
	demofile.seekg(0, std::ios::end);
	size_t size = demofile.tellg();
	std::string str(size, ' ');
	demofile.seekg(0);
	demofile.read(&str[0], size); 
	MemoryStreamBuffer demoBuffer(const_cast<char *>(str.c_str()), str.length());
	MemoryStream demo(demoBuffer);

	parseHeader(demo);

	int messageCount = 0;
	bool end = false;
	GameState gameState(0, demo.tellg());

	while (!end)
	{
		DemoParser demoParser(gameState);
		if (!demoParser.parseNextTick(demo))
		{
			end = true;
		}

		messageCount++;
	}

	std::cout << "message count: " << messageCount << std::endl;

	return 0;
}
