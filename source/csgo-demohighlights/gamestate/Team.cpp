#include "Team.h"

Team fromEngineInteger(int i)
{
	switch (i)
	{
	case 2:
		return Terrorists;
	case 3:
		return CounterTerrorists;
	}

	return UnknownTeam;
}

Team getOppositeTeam(Team team)
{
	switch (team)
	{
	case Terrorists:
		return CounterTerrorists;
	case CounterTerrorists:
		return Terrorists;
	}

	throw std::bad_exception("no opposite team");
}

std::string toString(Team team)
{
	switch (team)
	{
	case Terrorists:
		return "Terrorists";
	case CounterTerrorists:
		return "Counter-Terrorists";
	case Spectator:
		return "Spectator";
	case UnknownTeam:
	default:
		return "Unknown";
	}
}
