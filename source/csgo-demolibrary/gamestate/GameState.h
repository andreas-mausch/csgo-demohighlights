#pragma once

#include "Player.h"
#include "Round.h"
#include "../parser/DemoParser.h"
#include "../parser/Stringtable.h"
#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"
#include "../utils/PointerVector.h"
#include "../utils/Vector.h"

#include "../protobuf/generated/netmessages_public.pb.h"

#include <vector>

class MemoryStream;

class GameState
{
private:
	DemoHeader header;
	int tick;
	int continuousTick;
	int positionInStream;

	Round round;

	CSVCMsg_GameEventList gameEvents;
	std::vector<Stringtable> stringTables;
	std::vector<Player> players;
	std::vector<Team> teams;
	PointerVector<EntityEntry> s_Entities;

	int getTicksPerSecond();
	Player *findPlayerByEntityIdIfExists(int entitydId);

public:
	std::vector< CSVCMsg_SendTable > s_DataTables;
	std::vector<ServerClass> serverClasses;
	int s_nServerClassBits;
	std::vector< ExcludeEntry > s_currentExcludes;

public:
	GameState(int tick, int positionInStream);
	~GameState();

	const CSVCMsg_GameEventList::descriptor_t &getGameEvent(int eventId);
	std::vector<Stringtable> &getStringTables();

	DemoHeader &getHeader();
	void setHeader(DemoHeader header);

	int getPositionInStream();
	void setPositionInStream(int positionInStream);

	std::vector<Player> &getPlayers();
	Player &findPlayerByUserId(int userId);
	Player *findPlayerByUserIdIfExists(int userId);

	void updatePlayer(Player &player);
	void updatePlayerTeam(int entityId, TeamType team);
	void updatePlayerPositionXY(int entityId, float x, float y);
	void updatePlayerPositionZ(int entityId, float z);
	void updatePlayerObserverMode(int entityId, bool observer);
	void updatePlayerHealth(int entityId, int health);
	void updatePlayerEyeAngleX(int entityId, float x);
	void updatePlayerEyeAngleY(int entityId, float y);
	void updatePlayerPlanting(int userId, bool planting);
	void updatePlayerDefusing(int userId, bool defusing, bool kit);

	Team &getTeam(TeamType type);
	Team *getTeamIfExists(TeamType type);
	Team &getTeamByEntityId(int entityId);
	void addTeam(int entityId);
	void updateTeamScore(int entityId, int score);
	void updateTeamType(int entityId, TeamType type);
	void updateTeamname(int entityId, const std::string &name);
	int getRoundsWon(TeamType type);

	void disconnect(int userId);
	int getPlayersAlive(TeamType type);

	int getTick();
	void setTick(int tick);
	int getContinuousTick();
	void setContinuousTick(int tick);
	void setGameEvents(CSVCMsg_GameEventList &message);

	void setRoundTime(int seconds);
	void setRoundStartedTick(int tick);
	void setBombTimer(int seconds);
	int getBombPlantedTick();
	void setBombPlantedTick(int tick);
	int getBombTimeLeft();
	int getRoundTimeLeft();

	Vector getBombPosition();
	void setBombPosition(Vector position);

	PointerVector<EntityEntry> &getEntities();

};
