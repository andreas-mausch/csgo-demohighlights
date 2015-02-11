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
	std::map<int, int> headshotsPerPlayer;

	int tRoundsWon;
	int ctRoundsWon;

public:
	KillsFilter(GameState &gameState, Log &log);
	~KillsFilter();

	virtual void roundStart();
	virtual void playerDeath(Player &victim, Player &attacker, bool headshot);
	virtual void roundOfficiallyEnded();

};
