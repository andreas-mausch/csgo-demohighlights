#pragma once

#include <string>
#include "../gamestate/Team.h"
#include "../gamestate/Weapon.h"
#include "../utils/Vector.h"

class Player;

class GameEventHandler
{
public:
	GameEventHandler() {}
	virtual ~GameEventHandler() {}

	virtual void playerDeath(Player &victim, Player &attacker, bool headshot) {}
	virtual void bombPlanted() {}
	virtual void roundStart() {}
	virtual void roundFreezeEnd() {}
	virtual void roundEnd(TeamType winner) {}
	virtual void roundOfficiallyEnded() {}
	virtual void playerConnect(const std::string &name, int entityId, int userId) {}
	virtual void playerDisconnect(int userId) {}
	virtual void announcePhaseEnd() {}
	virtual void grenadeThrown(Weapon type, Player &player) {}
	virtual void grenadeDetonate(Weapon type, Player &player, Vector position) {}

};
