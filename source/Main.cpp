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

	int messageCount = 0;
	bool end = false;
	GameState gameState(0, demo.tellg());
	DemoParser demoParser(gameState);
	demoParser.parseHeader(demo);

	while (demoParser.parseNextTick(demo))
	{
		messageCount++;
	}

	std::cout << "message count: " << messageCount << std::endl;

	return 0;
}
