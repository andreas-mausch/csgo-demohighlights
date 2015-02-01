#pragma once

#include "Player.h"

#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"

#include "../protobuf/generated/netmessages_public.pb.h"

#include <vector>

class MemoryStream;

class GameState
{
private:
	int tick;
	int positionInStream;

	CSVCMsg_GameEventList gameEvents;
	std::vector<StringTableData_t> stringTables;
	std::vector<Player> players;

public:
	GameState(int tick, int positionInStream);
	~GameState();

	const CSVCMsg_GameEventList::descriptor_t &getGameEvent(int eventId);
	std::vector<StringTableData_t> &getStringTables();

	std::vector<Player> &getPlayers();
	Player &findPlayerByUserId(int userId);
	Player *findPlayerByUserIdIfExists(int userId);
	void updatePlayer(Player &player);

	void setTick(int tick);
	void setPositionInStream(int positionInStream);
	void setGameEvents(CSVCMsg_GameEventList &message);
};
