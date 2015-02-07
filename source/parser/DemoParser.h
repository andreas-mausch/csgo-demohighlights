#pragma once

class GameState;
class MemoryStream;
class MemoryBitStream;

class CSVCMsg_GameEvent;
class CSVCMsg_GameEventList;
class CSVCMsg_CreateStringTable;
class CSVCMsg_UpdateStringTable;

struct player_info_t;

#include <string>
#include <vector>

#include "../filters/GameEventHandler.h"
#include "../sdk/demofiledump.h"
#include "../utils/PointerVector.h"

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

class Log
{
public:
	virtual void log(const char *format, ...) = 0;
};

class DemoParser : public Log
{
private:
	GameState &gameState;

	bool verbose;
	std::ostream &outputStream;

	PointerVector<GameEventHandler> gameEventHandlers;

	// TODO remove / rename
	void serverInfo(const char *bytes, int length);
	void parsePacket2(MemoryStream &demo, int length);
	void gameEventList(CSVCMsg_GameEventList &message);
	void gameEvent(CSVCMsg_GameEvent &message);
	void createStringTable(CSVCMsg_CreateStringTable &message);
	void updateStringTable(CSVCMsg_UpdateStringTable &message);
	void parseStringtable(MemoryBitStream &stringtables);
	void parseStringTableUpdate(MemoryBitStream &stream, int entryCount, int maximumEntries, int userDataSize, int userDataSizeBits, int userDataFixedSize, bool userData);
	void unhandledCommand(const std::string &description);

	void updatePlayer(int entityId, const player_info_t *playerinfo);
	void updatePlayer(int entityId, int userId, const std::string &name);

	void logVargs(const std::string &format, va_list args);
	void logVerbose(const char *format, ...);

	void playerDeath(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void bombPlanted(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundStart(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundFreezeEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void playerConnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void playerDisconnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void announcePhaseEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);
	void roundOfficiallyEnded(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor);

public:
	DemoParser(GameState &gameState, bool verbose);
	~DemoParser();

	void parsePacket(MemoryStream &demo);
	void parseDatatables(MemoryStream &demo);
	void parseStringtables(MemoryStream &demo);

	DemoHeader parseHeader(MemoryStream &demo);

	bool parseNextTick(MemoryStream &demo);
	void log(const char *format, ...);

};
