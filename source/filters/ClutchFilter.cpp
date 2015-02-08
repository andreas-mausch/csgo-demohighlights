#include "ClutchFilter.h"
#include "../gamestate/GameState.h"
#include "../utils/Log.h"

ClutchFilter::ClutchFilter(GameState &gameState, Log &log)
: gameState(gameState), log(log)
{
}

ClutchFilter::~ClutchFilter()
{
}

void ClutchFilter::addPossibleClutch(Player &player)
{
	Clutch clutch;
	clutch.player = &player;
	clutch.aliveOwnTeam = gameState.getPlayersAlive(player.getTeam());
	clutch.aliveEnemyTeam = gameState.getPlayersAlive(getOppositeTeam(player.getTeam()));

	bool playerHasAClutchAlready = false;
	for (std::vector<Clutch>::iterator clutch = possibleClutches.begin(); clutch != possibleClutches.end(); clutch++)
	{
		if (clutch->player->getUserId() == player.getUserId())
		{
			playerHasAClutchAlready = true;
			break;
		}
	}

	if (!playerHasAClutchAlready)
	{
		possibleClutches.push_back(clutch);
	}
}

void ClutchFilter::playerDeath(Player &victim, Player &attacker, bool headshot)
{
	std::vector<Player> &players = gameState.getPlayers();

	if (gameState.getPlayersAlive(CounterTerrorists) == 1 && gameState.getPlayersAlive(Terrorists) > 0)
	{
		for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
		{
			if (player->isAlive() && player->getTeam() == CounterTerrorists)
			{
				addPossibleClutch(*player);
			}
		}
	}
	if (gameState.getPlayersAlive(Terrorists) == 1 && gameState.getPlayersAlive(CounterTerrorists) > 0)
	{
		for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
		{
			if (player->isAlive() && player->getTeam() == Terrorists)
			{
				addPossibleClutch(*player);
			}
		}
	}
}

void ClutchFilter::roundFreezeEnd()
{
	possibleClutches.clear();
}

void ClutchFilter::roundEnd(Team winner)
{
	for (std::vector<Clutch>::iterator clutch = possibleClutches.begin(); clutch != possibleClutches.end(); clutch++)
	{
		if (clutch->player->getTeam() == winner)
		{
			log.log("CLUTCH WON %d:%d - %dvs%d: %s (%s)", gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists), clutch->aliveOwnTeam, clutch->aliveEnemyTeam, clutch->player->getName().c_str(), toString(winner).c_str());
		}
	}
}
