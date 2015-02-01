#pragma once

#include <string>

enum Team
{
	CounterTerrorists,
	Terrorists,
	Spectator,
	Unknown
};

Team fromEngineInteger(int i);
std::string toString(Team team);
