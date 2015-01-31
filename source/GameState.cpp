#include "GameState.h"
#include "Main.h"

#include "sdk/demofile.h"
#include "streams/MemoryStream.h"
#include "utils/StringFormat.h"

GameState::GameState(int tick, int positionInStream)
	: tick(tick), positionInStream(positionInStream)
{
}

GameState::~GameState()
{
}

bool GameState::parseNextTick(MemoryStream &demo)
{
	unsigned char command = demo.readByte();
	tick = demo.readInt();
	unsigned char playerSlot = demo.readByte();

	switch (command)
	{
	case dem_signon:
	case dem_packet:
		parsePacket(demo);
		break;
	case dem_synctick:
		break;
	case dem_consolecmd:
		unhandledCommand(formatString("command: default %d", command));
		break;
	case dem_usercmd:
		unhandledCommand(formatString("command: default %d", command));
		break;
	case dem_datatables:
		parseDatatables(demo);
		break;
	case dem_stop:
		return false;
	case dem_customdata:
		unhandledCommand(formatString("command: default %d", command));
		break;
	case dem_stringtables:
		parseStringtables(demo);
		break;
	default:
		unhandledCommand(formatString("command: default %d", command));
	}

	positionInStream = demo.tellg();

	return true;
}
