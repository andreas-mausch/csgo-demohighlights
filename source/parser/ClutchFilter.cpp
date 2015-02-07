#include "ClutchFilter.h"
#include "DemoParser.h"
#include "../gamestate/GameState.h"

ClutchFilter::ClutchFilter(GameState &gameState, Log &log)
: clutch(NULL), clutchCount(-1), gameState(gameState), log(log)
{
}

ClutchFilter::~ClutchFilter()
{
}

void ClutchFilter::playerDeath(Player &victim, Player &attacker)
{
	if (clutch == NULL)
	{
		std::vector<Player> &players = gameState.getPlayers();

		if (gameState.getPlayersAlive(CounterTerrorists) == 1 && gameState.getPlayersAlive(Terrorists) > 0)
		{
			for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
			{
				if (player->isAlive() && player->getTeam() == CounterTerrorists)
				{
					clutch = &(*player);
					clutchCount = gameState.getPlayersAlive(Terrorists);
				}
			}
		}
		else if (gameState.getPlayersAlive(Terrorists) == 1 && gameState.getPlayersAlive(CounterTerrorists) > 0)
		{
			for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
			{
				if (player->isAlive() && player->getTeam() == Terrorists)
				{
					clutch = &(*player);
					clutchCount = gameState.getPlayersAlive(CounterTerrorists);
				}
			}
		}
	}
}

void ClutchFilter::roundFreezeEnd()
{
	clutch = NULL;
}

void ClutchFilter::roundEnd(Team winner)
{
	if (clutch && clutch->getTeam() == winner)
	{
		log.log("CLUTCH WON %d:%d - 1vs%d: %s (%s)", gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists), clutchCount, clutch->getName().c_str(), toString(winner).c_str());
	}
}
