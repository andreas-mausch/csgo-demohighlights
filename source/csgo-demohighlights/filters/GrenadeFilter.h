#pragma once

#include <vector>
#include "../../csgo-demolibrary/parser/GameEventHandler.h"

class GameState;
class Log;

class GrenadeFilter : public GameEventHandler
{
private:
	GameState &gameState;
	Log &log;

public:
	GrenadeFilter(GameState &gameState, Log &log);
	~GrenadeFilter();

	virtual void grenadeThrown(Weapon type, Player &player);
	virtual void grenadeDetonate(Weapon type, Player &player, Vector position);

};
