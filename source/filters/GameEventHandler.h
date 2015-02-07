#pragma once

#include <string>
#include "../gamestate/Team.h"

class Player;

class GameEventHandler
{
public:
	GameEventHandler() {}
	virtual ~GameEventHandler() {}

	virtual void playerDeath(Player &victim, Player &attacker) {}
	virtual void bombPlanted() {}
	virtual void roundStart() {}
	virtual void roundFreezeEnd() {}
	virtual void roundEnd(Team winner) {}
	virtual void roundOfficiallyEnded() {}
	virtual void playerConnect(const std::string &name, int entityId, int userId) {}
	virtual void playerDisconnect(int userId) {}
	virtual void announcePhaseEnd() {}

};
