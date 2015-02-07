#include "DemoParser.h"
#include "Datatable.h"
#include "Entities.h"
#include "Stringtable.h"
#include "../filters/ClutchFilter.h"
#include "../filters/KillsFilter.h"
#include "../filters/PlayerConnectHandler.h"
#include "../gamestate/GameState.h"
#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"
#include "../streams/MemoryStream.h"
#include "../streams/MemoryBitStream.h"
#include "../streams/MemoryStreamBuffer.h"
#include "../utils/EndianConverter.h"
#include "../utils/StringFormat.h"

DemoParser::DemoParser(GameState &gameState, bool verbose)
: gameState(gameState), log(std::cout, verbose)
{
	gameEventHandlers.push_back(new PlayerConnectHandler(gameState));
	gameEventHandlers.push_back(new ClutchFilter(gameState, log));
	gameEventHandlers.push_back(new KillsFilter(gameState, log));
}

DemoParser::~DemoParser()
{
}

DemoHeader DemoParser::parseHeader(MemoryStream &demo)
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

	return header;
}

void DemoParser::parsePacket(MemoryStream &demo)
{
	int position = demo.tellg();
	democmdinfo_t cmdinfo;
	demo.readBytes(&cmdinfo, sizeof(democmdinfo_t));
	int sequenceNumberIn = demo.readInt();
	int sequenceNumberOut = demo.readInt();
	int length = demo.readInt();
	log.logVerbose("Parse packet. Length: %d at %d / %d", length, position, demo.tellg());
	parsePacket2(demo, length);
}

void DemoParser::parseDatatables(MemoryStream &demo)
{
	int length = demo.readInt();
	log.logVerbose("Parse datatables: %d", length);
	char *datatablesBytes = new char[length];
	demo.readBytes(datatablesBytes, length);
	MemoryBitStream datatables(datatablesBytes, length);
	ParseDataTable(datatables);
	delete[] datatablesBytes;
}

void DemoParser::unhandledCommand(const std::string &description)
{
	log.log("ERROR Unhandled command: %s", description.c_str());
	throw std::bad_exception(description.c_str());
}

void DemoParser::serverInfo(const char *bytes, int length)
{
	CSVCMsg_ServerInfo serverInfo;
	serverInfo.ParseFromArray(bytes, length);
	delete[] bytes;
	log.logVerbose("serverInfo: %d, %s", length, serverInfo.DebugString().c_str());
}

void DemoParser::updatePlayer(int entityId, const player_info_t *playerinfo)
{
	int userId = endian_swap(playerinfo->userID);
	Player player(entityId, userId, playerinfo->name);
	gameState.updatePlayer(player);
	log.logVerbose("\tplayer name: %s / %d", playerinfo->name, userId);
}

bool DemoParser::parseNextTick(MemoryStream &demo)
{
	unsigned char command = demo.readByte();
	int tick = demo.readInt();
	gameState.setTick(tick);
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
		log.log("Game ended %d:%d", gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists));
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

	gameState.setPositionInStream(demo.tellg());

	log.logVerbose("GameState: %d / %d", tick, demo.tellg());

	return true;
}

void DemoParser::parsePacket2(MemoryStream &demo, int length)
{
	int destination = (int)demo.tellg() + length;

	while (demo.tellg() < destination)
	{
		int command = demo.readVarInt32();
		int messageLength = demo.readVarInt32();

		switch (command)
		{
		case net_Tick:
			demo.seekg(messageLength, std::ios_base::cur);
			break;
		case svc_ServerInfo:
		{
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			serverInfo(bytes, messageLength);
		} break;
		case svc_CreateStringTable:
		{
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_CreateStringTable message;
			message.ParseFromArray(bytes, messageLength);
			createStringTable(message);
		} break;
		case svc_UpdateStringTable:
		{
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_UpdateStringTable message;
			message.ParseFromArray(bytes, messageLength);
			updateStringTable(message);
		} break;
		case svc_GameEvent:
		{
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_GameEvent message;
			message.ParseFromArray(bytes, messageLength);
			gameEvent(message);
		} break;
		case svc_GameEventList:
		{
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_GameEventList message;
			message.ParseFromArray(bytes, messageLength);
			gameEventList(message);
		} break;
		case svc_PacketEntities:
		{
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_PacketEntities message;
			message.ParseFromArray(bytes, messageLength);
			packetEntities(message);
		} break;
		default:
			// unhandledCommand(formatString("message command: default %d", command));
			demo.seekg(messageLength, std::ios_base::cur);
		}
	}

	demo.seekg(destination, std::ios_base::beg);
}
