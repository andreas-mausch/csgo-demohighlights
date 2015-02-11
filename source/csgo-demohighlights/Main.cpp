#include <iostream>
#include <fstream>

#include "FilterHandler.h"
#include "../csgo-demolibrary/parser/DemoParser.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/streams/MemoryStreamBuffer.h"
#include "../csgo-demolibrary/streams/MemoryStream.h"

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
	FilterHandler filterHandler(gameState);
	DemoParser demoParser(gameState, false, filterHandler);
	demoParser.parseHeader(demo);

	while (demoParser.parseNextTick(demo))
	{
	}

	return 0;
}
