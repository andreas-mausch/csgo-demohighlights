#pragma once

#include <vector>
#include "../../csgo-demolibrary/parser/GameEventHandler.h"

class GameState;
class Log;

struct Clutch
{
	Player *player;
	int aliveOwnTeam;
	int aliveEnemyTeam;
};

class ClutchFilter : public GameEventHandler
{
private:
	std::vector<Clutch> possibleClutches;
	GameState &gameState;
	Log &log;

	void addPossibleClutch(Player &player);

public:
	ClutchFilter(GameState &gameState, Log &log);
	~ClutchFilter();

	virtual void playerDeath(Player &victim, Player &attacker, bool headshot);
	virtual void roundFreezeEnd();
	virtual void roundEnd(TeamType winner);

};
