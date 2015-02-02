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
std::string toString(Team team);
