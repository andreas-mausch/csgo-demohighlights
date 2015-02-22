#pragma once

class GameEventHandler;
class GameState;
class MemoryStream;
class MemoryBitStream;

class CSVCMsg_GameEvent;
class CSVCMsg_GameEventList;
class CSVCMsg_CreateStringTable;
class CSVCMsg_UpdateStringTable;

struct player_info_t;

#include <string>

#include "PlayerConnectHandler.h"
#include "../gamestate/Weapon.h"
#include "../sdk/demofiledump.h"
#include "../utils/PointerVector.h"
#include "../utils/Log.h"

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

class DemoParser
{
private:
	Log &log;
	GameState &gameState;

	GameEventHandler &gameEventHandler;
	PlayerConnectHandler playerConnectHandler;

	// TODO remove / rename
	void serverInfo(CSVCMsg_ServerInfo &message);
	void parsePacket2(MemoryStream &demo, int length);
	void gameEventList(CSVCMsg_GameEventList &message);
	void gameEvent(CSVCMsg_GameEvent &message);
	void createStringTable(CSVCMsg_CreateStringTable &message);
	void updateStringTable(CSVCMsg_UpdateStringTable &message);
	void packetEntities(CSVCMsg_PacketEntities &message);
	void parseStringtable(MemoryBitStream &stringtables);
	void parseStringTableUpdate(MemoryBitStream &stream, int entryCount, int maximumEntries, int userDataSize, int userDataSizeBits, int userDataFixedSize, bool userData);
	void unhandledCommand(const std::string &description);

	void updatePlayer(int entityId, const player_info_t *playerinfo);
	void updatePlayer(int entityId, int userId, const std::string &name);

	void playerDeath(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void bombPlanted(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundStart(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundFreezeEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void playerConnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void playerDisconnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void announcePhaseEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundOfficiallyEnded(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void weaponFire(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void grenadeDetonate(Weapon type, CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);

	EntityEntry *AddEntity(int nEntity, uint32 uClass, uint32 uSerialNum);
	bool ReadNewEntity(MemoryBitStream &entityBitBuffer, EntityEntry *pEntity);

	template<typename T> void parseMessage(MemoryStream &demo, int length, void (DemoParser::*function)(T &))
	{
		char *bytes = new char[length];
		demo.readBytes(bytes, length);
		T message;
		message.ParseFromArray(bytes, length);
		delete[] bytes;
		(this->*function)(message);
	}

public:
	DemoParser(GameState &gameState, Log &log, GameEventHandler &gameEventHandler);
	~DemoParser();

	void parsePacket(MemoryStream &demo);
	void parseDatatables(MemoryStream &demo);
	void parseStringtables(MemoryStream &demo);

	DemoHeader parseHeader(MemoryStream &demo);

	bool parseNextTick(MemoryStream &demo);

};
