#pragma once

#include "GameEventHandler.h"

class GameState;
class Log;

class ClutchFilter : public GameEventHandler
{
private:
	Player *clutch;
	int clutchCount;
	GameState &gameState;
	Log &log;

public:
	ClutchFilter(GameState &gameState, Log &log);
	~ClutchFilter();

	virtual void playerDeath(Player &victim, Player &attacker);
	virtual void roundFreezeEnd();
	virtual void roundEnd(Team winner);

};
