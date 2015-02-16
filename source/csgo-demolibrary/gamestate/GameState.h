#pragma once

#include "Player.h"
#include "../parser/Stringtable.h"

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

	int tRoundsWon;
	int ctRoundsWon;

	CSVCMsg_GameEventList gameEvents;
	std::vector<Stringtable> stringTables;
	std::vector<Player> players;

public:
	GameState(int tick, int positionInStream);
	~GameState();

	const CSVCMsg_GameEventList::descriptor_t &getGameEvent(int eventId);
	std::vector<Stringtable> &getStringTables();

	std::vector<Player> &getPlayers();
	Player &findPlayerByUserId(int userId);
	Player *findPlayerByUserIdIfExists(int userId);
	void updatePlayer(Player &player);
	void disconnect(int userId);
	int getPlayersAlive(Team team);

	void switchTeams();

	int getTick();
	void setTick(int tick);
	void setPositionInStream(int positionInStream);
	void setGameEvents(CSVCMsg_GameEventList &message);

	int getRoundsWon(Team team);
	void addWonRound(Team team);
};