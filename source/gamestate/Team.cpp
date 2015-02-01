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

	return Unknown;
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
	case Unknown:
	default:
		return "Unknown";
	}
}
