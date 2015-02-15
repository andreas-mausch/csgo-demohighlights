#include "FilterHandler.h"
#include "filters/ClutchFilter.h"
#include "filters/GrenadeFilter.h"
#include "filters/KillsFilter.h"

FilterHandler::FilterHandler(GameState &gameState, Log &log)
: gameState(gameState)
{
	handlers.push_back(new ClutchFilter(gameState, log));
	handlers.push_back(new GrenadeFilter(gameState, log));
	handlers.push_back(new KillsFilter(gameState, log));
}

FilterHandler::~FilterHandler()
{
}

void FilterHandler::playerDeath(Player &victim, Player &attacker, bool headshot)
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->playerDeath(victim, attacker, headshot);
	}
}

void FilterHandler::bombPlanted()
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->bombPlanted();
	}
}

void FilterHandler::roundStart()
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->roundStart();
	}
}

void FilterHandler::roundFreezeEnd()
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->roundFreezeEnd();
	}
}

void FilterHandler::roundEnd(Team winner)
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->roundEnd(winner);
	}
}

void FilterHandler::roundOfficiallyEnded()
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->roundOfficiallyEnded();
	}
}

void FilterHandler::playerConnect(const std::string &name, int entityId, int userId)
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->playerConnect(name, entityId, userId);
	}
}

void FilterHandler::playerDisconnect(int userId)
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->playerDisconnect(userId);
	}
}

void FilterHandler::announcePhaseEnd()
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->announcePhaseEnd();
	}
}

void FilterHandler::grenadeThrown(Weapon type, Player &player)
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->grenadeThrown(type, player);
	}
}

void FilterHandler::grenadeDetonate(Weapon type, Player &player, Vector position)
{
	for (PointerVector<GameEventHandler>::iterator handler = handlers.begin(); handler != handlers.end(); handler++)
	{
		(*handler)->grenadeDetonate(type, player, position);
	}
}
