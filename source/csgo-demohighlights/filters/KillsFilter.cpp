#include "KillsFilter.h"
#include "../../csgo-demolibrary/gamestate/GameState.h"
#include "../../csgo-demolibrary/utils/Log.h"

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
			log.log("%1dK (%1d HS)   %02d:%02d %-15.15s %s", it->second, headshotsPerPlayer[player.getUserId()], tRoundsWon, ctRoundsWon, player.getName().c_str(), Team::toString(player.getTeam()).c_str());
		}
	}

	killsPerPlayer.clear();
	headshotsPerPlayer.clear();
}
