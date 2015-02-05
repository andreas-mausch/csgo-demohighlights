#pragma once

#include <string>

enum Team
{
	CounterTerrorists,
	Terrorists,
	Spectator,
	UnknownTeam
};

Team fromEngineInteger(int i);
Team getOppositeTeam(Team team);
std::string toString(Team team);
