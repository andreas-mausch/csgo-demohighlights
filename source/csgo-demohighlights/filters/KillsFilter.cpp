#include "KillsFilter.h"
#include "../gamestate/GameState.h"
#include "../utils/Log.h"

KillsFilter::KillsFilter(GameState &gameState, Log &log)
: gameState(gameState), log(log), tRoundsWon(-1), ctRoundsWon(-1)
{
}

KillsFilter::~KillsFilter()
{
}

void KillsFilter::roundStart()
{
	tRoundsWon = gameState.getRoundsWon(Terrorists);
	ctRoundsWon = gameState.getRoundsWon(CounterTerrorists);
}

void KillsFilter::playerDeath(Player &victim, Player &attacker, bool headshot)
{
	killsPerPlayer[attacker.getUserId()]++;

	if (headshot)
	{
		headshotsPerPlayer[attacker.getUserId()]++;
	}
}

void KillsFilter::roundOfficiallyEnded()
{
	for (std::map<int, int>::iterator it = killsPerPlayer.begin(); it != killsPerPlayer.end(); it++)
	{
		if (it->second >= 3)
		{
			Player &player = gameState.findPlayerByUserId(it->first);
			log.log("KILLS %d:%d - %dK (%d Headshots) %s (%s)", tRoundsWon, ctRoundsWon, it->second, headshotsPerPlayer[player.getUserId()], player.getName().c_str(), toString(player.getTeam()).c_str());
		}
	}

	killsPerPlayer.clear();
	headshotsPerPlayer.clear();
}
