#pragma once

class GameEventHandler
{

	virtual void playerDeath(Player &victim, Player &attacker) {}
	virtual void bombPlanted() {}
	virtual void roundStart() {}
	virtual void roundFreezeEnd() {}
	virtual void roundEnd() {}
	virtual void playerConnect() {}
	virtual void playerDisconnect() {}
	virtual void announcePhaseEnd() {}

};
