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
	GameState &gameState;

	// TODO remove / rename
	void parsePacket2(MemoryStream &demo, int length);
	void gameEventList(CSVCMsg_GameEventList &message);
	void gameEvent(CSVCMsg_GameEvent &message);
	void createStringTable(CSVCMsg_CreateStringTable &message);
	void updateStringTable(CSVCMsg_UpdateStringTable &message);
	void parseStringtable(MemoryBitStream &stringtables);
	void parseStringTableUpdate(MemoryBitStream &stream, int entryCount, int maximumEntries, int userDataSize, int userDataSizeBits, int userDataFixedSize, bool userData);

	void updatePlayer(int entityId, const player_info_t *playerinfo);
	void updatePlayer(int entityId, int userId, const std::string &name);

public:
	DemoParser(GameState &gameState);
	~DemoParser();

	void parsePacket(MemoryStream &demo);
	void parseDatatables(MemoryStream &demo);
	void parseStringtables(MemoryStream &demo);

	DemoHeader parseHeader(MemoryStream &demo);

	bool parseNextTick(MemoryStream &demo);
};
