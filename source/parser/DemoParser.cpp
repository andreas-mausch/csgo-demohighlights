#include <sstream>
#include <stdarg.h>

#include "DemoParser.h"
#include "Datatable.h"
#include "Entities.h"
#include "Stringtable.h"
#include "../gamestate/GameState.h"

#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"

#include "../streams/MemoryStream.h"
#include "../streams/MemoryBitStream.h"
#include "../streams/MemoryStreamBuffer.h"

#include "../utils/EndianConverter.h"
#include "../utils/StringFormat.h"

void DemoParser::log(bool verbose, const std::string &format, ...)
{
	if (!this->verbose || verbose)
	{
		va_list args;
		va_start(args, format);
		std::string text = formatString(format, args);
		va_end(args);
		outputStream << "info: " << text << std::endl;
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
	log(true, "serverInfo: %d, %s", length, serverInfo.DebugString().c_str());
}

void DemoParser::updatePlayer(int entityId, const player_info_t *playerinfo)
{
	int userId = endian_swap(playerinfo->userID);
	Player player(entityId, userId, playerinfo->name);
	gameState.updatePlayer(player);
	log(true, "\tplayer name: %s / %d", playerinfo->name, userId);
}

void DemoParser::updatePlayer(int entityId, int userId, const std::string &name)
{
	Player player(entityId, userId, name);
	gameState.updatePlayer(player);
	log(true, "\tplayer name: %s / %d", name.c_str(), userId);
}

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

	throw 2;
}

std::string toString(Player &player)
{
	std::stringstream output;
	output << player.getName() << " (" << toString(player.getTeam()) << ")";
	return output.str();
}

int roundStart = -1;
Player *clutch = NULL;
int clutchCount = 0;

void DemoParser::gameEvent(CSVCMsg_GameEvent &message)
{
	const CSVCMsg_GameEventList::descriptor_t& descriptor = gameState.getGameEvent(message.eventid());
	std::vector<Player> &players = gameState.getPlayers();
	log(true, "gameEvent: %s" , descriptor.name().c_str());

	if (descriptor.name() == "player_death")
	{
		int tickDif = gameState.getTick() - roundStart;
		Player &attacker = gameState.findPlayerByUserId(getValue(message, descriptor, "attacker").val_short());
		Player &userid = gameState.findPlayerByUserId(getValue(message, descriptor, "userid").val_short());
		userid.setAlive(false);
		log(true, "player_death: %s killed %s", toString(attacker).c_str(), toString(userid).c_str());

		if (clutch == NULL)
		{
			if (gameState.getPlayersAlive(CounterTerrorists) == 1 && gameState.getPlayersAlive(Terrorists) > 0)
			{
				for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
				{
					if (player->isAlive() && player->getTeam() == CounterTerrorists)
					{
						clutch = &(*player);
						clutchCount = gameState.getPlayersAlive(Terrorists);
					}
				}
			}
			else if (gameState.getPlayersAlive(Terrorists) == 1 && gameState.getPlayersAlive(CounterTerrorists) > 0)
			{
				for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
				{
					if (player->isAlive() && player->getTeam() == Terrorists)
					{
						clutch = &(*player);
						clutchCount = gameState.getPlayersAlive(CounterTerrorists);
					}
				}
			}
		}
	}
	else if (descriptor.name() == "bomb_planted")
	{
	}
	else if (descriptor.name() == "round_freeze_end")
	{
		log(true, "timelimit: %d; tick: %d", getValue(message, descriptor, "timelimit").val_long(), gameState.getTick());
		roundStart = gameState.getTick();
		clutch = NULL;

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
			log(true, "\t%s", toString(*player).c_str());
		}
	}
	else if (descriptor.name() == "round_end")
	{
		Team winner = fromEngineInteger(getValue(message, descriptor, "winner").val_byte());
		log(true, "%s %d:%d", toString(winner).c_str(), gameState.getPlayersAlive(Terrorists), gameState.getPlayersAlive(CounterTerrorists));

		if (clutch && clutch->getTeam() == winner)
		{
			log(false, "CLUTCH WON %d:%d - 1vs%d: %s; %s", gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists), clutchCount, clutch->getName().c_str(), toString(winner).c_str());
		}

		gameState.addWonRound(winner);
	}
	else if (descriptor.name() == "player_connect")
	{
		std::string name = getValue(message, descriptor, "name").val_string();
		int entityId = getValue(message, descriptor, "index").val_byte() + 1;
		int userId = getValue(message, descriptor, "userid").val_short();
		updatePlayer(entityId, userId, name);
	}
	else if (descriptor.name() == "player_disconnect")
	{
		int userId = getValue(message, descriptor, "userid").val_short();
		gameState.disconnect(userId);
	}
	else if (descriptor.name() == "announce_phase_end")
	{
		// std::cout << "gameEvent: " << descriptor.name() << std::endl;
		gameState.switchTeams();
	}
}

void DemoParser::gameEventList(CSVCMsg_GameEventList &message)
{
	// std::cout << "gameEventList" << std::endl;
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
	// std::cout << "tableName: " << tableName << std::endl;

	int wordCount = stringtables.readWord();
	bool userInfo = tableName == "userinfo";
	// std::cout << "wordCount: " << wordCount << std::endl;

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
			// std::cout << "\tname: " << name << std::endl;

			bool hasUserData = stringtables.readBit();
			if (hasUserData)
			{
				int userDataLength = stringtables.readWord();
				unsigned char *data = new unsigned char[ userDataLength + 4 ];
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
	// std::cout << "Parse packet. Length: " << length << " at " << position << " / " << demo.tellg() << std::endl;
	parsePacket2(demo, length);
}

void DemoParser::parseDatatables(MemoryStream &demo)
{
	int length = demo.readInt();
	// std::cout << "Parse datatables: " << length << std::endl;
	char *datatablesBytes = new char[length];
	demo.readBytes(datatablesBytes, length);
	MemoryBitStream datatables(datatablesBytes, length);
	ParseDataTable(datatables);
	delete[] datatablesBytes;
}

void DemoParser::parseStringtables(MemoryStream &demo)
{
	int length = demo.readInt();
	// std::cout << "Parse stringtables: " << length << std::endl;
	char *stringtablesBytes = new char[length];
	demo.readBytes(stringtablesBytes, length);
	MemoryBitStream stringtables(stringtablesBytes, length);
	int tableCount = stringtables.readByte();
	// std::cout << "Parse stringtables tableCount: " << tableCount << std::endl;

	for (int i = 0; i < tableCount; i++)
	{
		parseStringtable(stringtables);
	}

	delete[] stringtablesBytes;
}

bool DemoParser::parseNextTick(MemoryStream &demo)
{
	unsigned char command = demo.readByte();
	gameState.setTick(demo.readInt());
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
		std::cout << "Game ended " << gameState.getRoundsWon(Terrorists) << ":" << gameState.getRoundsWon(CounterTerrorists) << std::endl;
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

	// std::cout << "GameState: " << tick << " / " << positionInStream << std::endl;

	return true;
}
