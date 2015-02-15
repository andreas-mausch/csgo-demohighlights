#include "GrenadeFilter.h"
#include "../../csgo-demolibrary/gamestate/GameState.h"
#include "../../csgo-demolibrary/utils/Log.h"

GrenadeFilter::GrenadeFilter(GameState &gameState, Log &log)
: gameState(gameState), log(log)
{
}

GrenadeFilter::~GrenadeFilter()
{
}

void GrenadeFilter::grenadeThrown(Weapon type, Player &player)
{
	log.logVerbose("grenade thrown: %d, %s", type, player.getName().c_str());
}

void GrenadeFilter::grenadeDetonate(Weapon type, Player &player, Vector position)
{
	log.logVerbose("grenade detonate: %d, %s, (%.2f, %.2f, %.2f)", type, player.getName().c_str(), position.x, position.y, position.z);
}
