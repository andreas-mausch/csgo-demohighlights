#pragma once

class GameState;
class MemoryStream;

class CSVCMsg_GameEvent;
class CSVCMsg_GameEventList;

class DemoParser
{
private:
	GameState &gameState;

	// TODO remove / rename
	void parsePacket2(MemoryStream &demo, int length);
	void gameEventList(CSVCMsg_GameEventList &message);
	void gameEvent(CSVCMsg_GameEvent &message);

public:
	DemoParser(GameState &gameState);
	~DemoParser();

	void parsePacket(MemoryStream &demo);
	void parseDatatables(MemoryStream &demo);
	void parseStringtables(MemoryStream &demo);

	bool parseNextTick(MemoryStream &demo);
};
