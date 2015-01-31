#pragma once

#include "sdk/demofile.h"
#include "sdk/demofiledump.h"

#include "protobuf/generated/netmessages_public.pb.h"

#include <vector>

class MemoryStream;

class GameState
{
private:
	int tick;
	int positionInStream;

	CSVCMsg_GameEventList gameEvents;
	std::vector<StringTableData_t> stringTables;

public:
	GameState(int tick, int positionInStream);
	~GameState();

	const CSVCMsg_GameEventList::descriptor_t &getGameEvent(int eventId);
	std::vector<StringTableData_t> &getStringTables();

	void setTick(int tick);
	void setPositionInStream(int positionInStream);
	void setGameEvents(CSVCMsg_GameEventList &message);
};
