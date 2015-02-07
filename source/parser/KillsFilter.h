#pragma once

#include "GameEventHandler.h"
#include <map>

class GameState;
class Log;

class KillsFilter : public GameEventHandler
{
private:
	GameState &gameState;
	Log &log;
	std::map<int, int> killsPerPlayer;

public:
	KillsFilter(GameState &gameState, Log &log);
	~KillsFilter();

	virtual void playerDeath(Player &victim, Player &attacker);
	virtual void roundFreezeEnd();

};
