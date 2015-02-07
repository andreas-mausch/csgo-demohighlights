#include <sstream>
#include <stdarg.h>

#include "ClutchFilter.h"
#include "DemoParser.h"
#include "Datatable.h"
#include "Entities.h"
#include "PlayerConnectHandler.h"
#include "Stringtable.h"
#include "../gamestate/GameState.h"

#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"

#include "../streams/MemoryStream.h"
#include "../streams/MemoryBitStream.h"
#include "../streams/MemoryStreamBuffer.h"

#include "../utils/EndianConverter.h"
#include "../utils/StringFormat.h"

const CSVCMsg_GameEvent::key_t& getValue(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor, const std::string &keyName)
{
	for (int i = 0; i < descriptor.keys().size(); i++)
	{
		const CSVCMsg_GameEventList::key_t& key = descriptor.keys(i);
		if (key.name() == keyName)
		{
			return message.keys(i);
		}
	}

	throw std::bad_exception("key not found");
}

std::string toString(Player &player)
{
	std::stringstream output;
	output << player.getName() << " (" << toString(player.getTeam()) << ")";
	return output.str();
}

int roundStart = -1;

void DemoParser::gameEvent(CSVCMsg_GameEvent &message)
{
	const CSVCMsg_GameEventList::descriptor_t& descriptor = gameState.getGameEvent(message.eventid());
	std::vector<Player> &players = gameState.getPlayers();
	logVerbose("gameEvent: %s" , descriptor.name().c_str());

	if (descriptor.name() == "player_death")
	{
		int tickDif = gameState.getTick() - roundStart;
		Player &attacker = gameState.findPlayerByUserId(getValue(message, descriptor, "attacker").val_short());
		Player &userid = gameState.findPlayerByUserId(getValue(message, descriptor, "userid").val_short());
		userid.setAlive(false);

		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->playerDeath(userid, attacker);
		}

		logVerbose("player_death: %s killed %s", toString(attacker).c_str(), toString(userid).c_str());
	}
	else if (descriptor.name() == "bomb_planted")
	{
		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->bombPlanted();
		}
	}
	else if (descriptor.name() == "round_start")
	{
		logVerbose("timelimit: %d; tick: %d", getValue(message, descriptor, "timelimit").val_long(), gameState.getTick());
		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->roundStart();
		}
	}
	else if (descriptor.name() == "round_freeze_end")
	{
		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->roundFreezeEnd();
		}

		roundStart = gameState.getTick();

		for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
		{
			int entityId = player->getEntityId();
			EntityEntry *entity = FindEntity(entityId);
			Team team = UnknownTeam;

			if (entity)
			{
				PropEntry *prop = entity->FindProp("m_iTeamNum");

				if (prop)
				{
					int teamInt = prop->m_pPropValue->m_value.m_int;
					team = fromEngineInteger(teamInt);
				}
			}

			player->setTeam(team);
			player->setAlive(true);
			logVerbose("\t%s", toString(*player).c_str());
		}
	}
	else if (descriptor.name() == "round_end")
	{
		Team winner = fromEngineInteger(getValue(message, descriptor, "winner").val_byte());
		logVerbose("%s %d:%d", toString(winner).c_str(), gameState.getPlayersAlive(Terrorists), gameState.getPlayersAlive(CounterTerrorists));
		gameState.addWonRound(winner);

		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->roundEnd(winner);
		}
	}
	else if (descriptor.name() == "player_connect")
	{
		std::string name = getValue(message, descriptor, "name").val_string();
		int entityId = getValue(message, descriptor, "index").val_byte() + 1;
		int userId = getValue(message, descriptor, "userid").val_short();

		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->playerConnect(name, entityId, userId);
		}
	}
	else if (descriptor.name() == "player_disconnect")
	{
		int userId = getValue(message, descriptor, "userid").val_short();
		for (std::vector<GameEventHandler *>::iterator handler = gameEventHandlers.begin(); handler != gameEventHandlers.end(); handler++)
		{
			(*handler)->playerDisconnect(userId);
		}
	}
	else if (descriptor.name() == "announce_phase_end")
	{
		gameState.switchTeams();
	}
}

void DemoParser::gameEventList(CSVCMsg_GameEventList &message)
{
	logVerbose("gameEventList");
	gameState.setGameEvents(message);
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
		case svc_ServerInfo: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			serverInfo(bytes, messageLength);
							 } break;
		case svc_CreateStringTable: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_CreateStringTable message;
			message.ParseFromArray(bytes, messageLength);
			createStringTable(message);
									} break;
		case svc_UpdateStringTable: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_UpdateStringTable message;
			message.ParseFromArray(bytes, messageLength);
			updateStringTable(message);
									} break;
		case svc_GameEvent: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_GameEvent message;
			message.ParseFromArray(bytes, messageLength);
			gameEvent(message);
							} break;
		case svc_GameEventList: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_GameEventList message;
			message.ParseFromArray(bytes, messageLength);
			gameEventList(message);
								} break;
		case svc_PacketEntities: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_PacketEntities message;
			message.ParseFromArray(bytes, messageLength);
			packetEntities(message);
								 } break;
		default:
			// unhandledCommand(string_format("message command: default %d", command));
			demo.seekg(messageLength, std::ios_base::cur);
		}
	}

	demo.seekg(destination, std::ios_base::beg);
}

void DemoParser::parseStringtable(MemoryBitStream &stringtables)
{
	std::string tableName = stringtables.readNullTerminatedString(256);
	int wordCount = stringtables.readWord();
	bool userInfo = tableName == "userinfo";

	logVerbose("parseStringtable: %s / %d", tableName.c_str(), wordCount);

	for (int i = 0; i < wordCount; i++)
	{
		std::string name = stringtables.readNullTerminatedString(4096);

		bool hasUserData = stringtables.readBit();
		if (hasUserData)
		{
			int userDataLength = stringtables.readWord();
			unsigned char *data = new unsigned char[ userDataLength + 4 ];
			stringtables.readBytes(data, userDataLength);

			if (userInfo)
			{
				updatePlayer(i + 1, reinterpret_cast<const player_info_t *>(data));
			}

			delete[] data;
		}
		else
		{
		}
	}

	bool clientSideData = stringtables.readBit();

	if (clientSideData)
	{
		for (int i = 0; i < wordCount; i++)
		{
			std::string name = stringtables.readNullTerminatedString(4096);
			logVerbose("\tname: ", name.c_str());

			bool hasUserData = stringtables.readBit();
			if (hasUserData)
			{
				int userDataLength = stringtables.readWord();
				unsigned char *data = new unsigned char[userDataLength + 4];
				stringtables.readBytes(data, userDataLength);
				delete[] data;
			}
			else
			{
			}
		}
	}
}

DemoParser::DemoParser(GameState &gameState, bool verbose)
: gameState(gameState), verbose(verbose), outputStream(std::cout)
{
	gameEventHandlers.push_back(new PlayerConnectHandler(gameState));
	gameEventHandlers.push_back(new ClutchFilter(gameState, *this));
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
	logVerbose("Parse packet. Length: %d at %d / %d", length, position, demo.tellg());
	parsePacket2(demo, length);
}

void DemoParser::parseDatatables(MemoryStream &demo)
{
	int length = demo.readInt();
	logVerbose("Parse datatables: %d", length);
	char *datatablesBytes = new char[length];
	demo.readBytes(datatablesBytes, length);
	MemoryBitStream datatables(datatablesBytes, length);
	ParseDataTable(datatables);
	delete[] datatablesBytes;
}

void DemoParser::parseStringtables(MemoryStream &demo)
{
	int length = demo.readInt();
	logVerbose("parseStringtables: %d", length);
	char *stringtablesBytes = new char[length];
	demo.readBytes(stringtablesBytes, length);
	MemoryBitStream stringtables(stringtablesBytes, length);
	int tableCount = stringtables.readByte();
	logVerbose("Parse stringtables tableCount: %d", tableCount);

	for (int i = 0; i < tableCount; i++)
	{
		parseStringtable(stringtables);
	}

	delete[] stringtablesBytes;
}

void DemoParser::log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	logVargs(format, args);
	va_end(args);
}

void DemoParser::logVargs(const std::string &format, va_list args)
{
	outputStream << "info: " << formatStringVargs(format, args) << std::endl;
}

void DemoParser::logVerbose(const char *format, ...)
{
	if (this->verbose)
	{
		va_list args;
		va_start(args, format);
		logVargs(format, args);
		va_end(args);
	}
}

void DemoParser::unhandledCommand(const std::string &description)
{
	outputStream << "ERROR Unhandled command: " << description << std::endl;
	throw std::bad_exception(description.c_str());
}

void DemoParser::serverInfo(const char *bytes, int length)
{
	CSVCMsg_ServerInfo serverInfo;
	serverInfo.ParseFromArray(bytes, length);
	delete[] bytes;
	logVerbose("serverInfo: %d, %s", length, serverInfo.DebugString().c_str());
}

void DemoParser::updatePlayer(int entityId, const player_info_t *playerinfo)
{
	int userId = endian_swap(playerinfo->userID);
	Player player(entityId, userId, playerinfo->name);
	gameState.updatePlayer(player);
	logVerbose("\tplayer name: %s / %d", playerinfo->name, userId);
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
		log("Game ended %d:%d", gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists));
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

	logVerbose("GameState: %d / %d", tick, demo.tellg());

	return true;
}
