#pragma once

#include "../csgo-demolibrary/parser/GameEventHandler.h"
#include "../csgo-demolibrary/utils/PointerVector.h"

class GameState;
class Log;

class FilterHandler : public GameEventHandler
{
private:
	PointerVector<GameEventHandler> handlers;
	GameState &gameState;

public:
	FilterHandler(GameState &gameState, Log &log);
	virtual ~FilterHandler();

	virtual void playerDeath(Player &victim, Player &attacker, bool headshot);
	virtual void bombPlanted();
	virtual void roundStart();
	virtual void roundFreezeEnd();
	virtual void roundEnd(Team winner);
	virtual void roundOfficiallyEnded();
	virtual void playerConnect(const std::string &name, int entityId, int userId);
	virtual void playerDisconnect(int userId);
	virtual void announcePhaseEnd();

};
