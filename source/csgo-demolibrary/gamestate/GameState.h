#pragma once

#include "Player.h"
#include "../parser/Stringtable.h"
#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"
#include "../utils/Vector.h"

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

	Player *findPlayerByEntityIdIfExists(int entitydId);

public:
	GameState(int tick, int positionInStream);
	~GameState();

	const CSVCMsg_GameEventList::descriptor_t &getGameEvent(int eventId);
	std::vector<Stringtable> &getStringTables();

	std::vector<Player> &getPlayers();
	Player &findPlayerByUserId(int userId);
	Player *findPlayerByUserIdIfExists(int userId);

	void updatePlayer(Player &player);
	void updatePlayerTeam(int entityId, Team team);
	void updatePlayerPositionXY(int entityId, float x, float y);
	void updatePlayerPositionZ(int entityId, float z);

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
