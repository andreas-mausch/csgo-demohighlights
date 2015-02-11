#include "FilterHandler.h"

FilterHandler::FilterHandler(GameState &gameState)
: gameState(gameState)
{
}

FilterHandler::~FilterHandler()
{
}

void FilterHandler::playerDeath(Player &victim, Player &attacker, bool headshot)
{
}

void FilterHandler::bombPlanted()
{
}

void FilterHandler::roundStart()
{
}

void FilterHandler::roundFreezeEnd()
{
}

void FilterHandler::roundEnd(Team winner)
{
}

void FilterHandler::roundOfficiallyEnded()
{
}

void FilterHandler::playerConnect(const std::string &name, int entityId, int userId)
{
}

void FilterHandler::playerDisconnect(int userId)
{
}

void FilterHandler::announcePhaseEnd()
{
}
