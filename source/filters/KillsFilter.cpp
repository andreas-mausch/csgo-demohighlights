#include "KillsFilter.h"
#include "../gamestate/GameState.h"
#include "../utils/Log.h"

KillsFilter::KillsFilter(GameState &gameState, Log &log)
: gameState(gameState), log(log)
{
}

KillsFilter::~KillsFilter()
{
}

void KillsFilter::playerDeath(Player &victim, Player &attacker)
{
	killsPerPlayer[attacker.getUserId()]++;
}

void KillsFilter::roundOfficiallyEnded()
{
	for (std::map<int, int>::iterator it = killsPerPlayer.begin(); it != killsPerPlayer.end(); it++)
	{
		if (it->second >= 3)
		{
			Player &player = gameState.findPlayerByUserId(it->first);
			log.log("KILLS %d:%d - %dk %s (%s)", gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists), it->second, player.getName().c_str(), toString(player.getTeam()).c_str());
		}
	}

	killsPerPlayer.clear();
}
