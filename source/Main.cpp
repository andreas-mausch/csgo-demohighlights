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

#include "parser/DemoParser.h"
#include "gamestate/GameState.h"

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string buffer(size, ' ');
	file.seekg(0);
	file.read(&buffer[0], size);
	return buffer;
}

int main()
{
	const std::string filename = "demo.dem";
	std::string stringBuffer = readFile(filename);
	MemoryStreamBuffer demoBuffer(const_cast<char *>(stringBuffer.c_str()), stringBuffer.length());
	MemoryStream demo(demoBuffer);

	bool end = false;
	GameState gameState(0, demo.tellg());
	DemoParser demoParser(gameState, false);
	demoParser.parseHeader(demo);

	while (demoParser.parseNextTick(demo))
	{
	}

	return 0;
}
