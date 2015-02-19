#include "FilterHandler.h"
#include "../csgo-demolibrary/parser/DemoParser.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/streams/MemoryStreamBuffer.h"
#include "../csgo-demolibrary/streams/MemoryStream.h"
#include "../csgo-demolibrary/utils/File.h"

int main()
{
	const std::string filename = "demo.dem";
	std::string stringBuffer = readFile(filename);

	if (stringBuffer.length() == 0)
	{
		std::cout << "error reading file." << std::endl;
		return -1;
	}

	MemoryStreamBuffer demoBuffer(const_cast<char *>(stringBuffer.c_str()), stringBuffer.length());
	MemoryStream demo(demoBuffer);

	bool end = false;
	GameState gameState(0, demo.tellg());
	Log log(std::cout, false);
	FilterHandler filterHandler(gameState, log);
	DemoParser demoParser(gameState, log, filterHandler);
	demoParser.parseHeader(demo);

	while (demoParser.parseNextTick(demo))
	{
	}

	return 0;
}
