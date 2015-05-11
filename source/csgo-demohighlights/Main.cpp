#include "FilterHandler.h"
#include "../csgo-demolibrary/parser/DemoParser.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/streams/MemoryStreamBuffer.h"
#include "../csgo-demolibrary/streams/MemoryStream.h"
#include "../csgo-demolibrary/utils/File.h"
#include "../csgo-demolibrary/utils/StringFormat.h"

void printHeader()
{
	std::cout << formatString("%-11.11s %5.5s %-15.15s %s", "Type", "Round", "Player", "Team") << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "please provide demo file: " << argv[0] << " demo.dem" << std::endl;
		return -1;
	}

	std::string stringBuffer = readFile(argv[1]);
	if (stringBuffer.length() == 0)
	{
		std::cout << "error reading file." << std::endl;
		return -1;
	}

	printHeader();

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
