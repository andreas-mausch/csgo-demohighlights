#pragma once

#include <string>

enum TeamType
{
	CounterTerrorists,
	Terrorists,
	Spectator,
	UnknownTeam
};

class Team
{
private:
	int entityId;
	TeamType type;
	int score;
	std::string name;

public:

	Team(int entityId);
	~Team();

	int getEntityId();

	int getScore();
	void setScore(int score);

	const std::string &getName();
	void setName(const std::string &name);

	TeamType getType();
	void setType(TeamType type);

	static TeamType fromEngineInteger(int i);
	static TeamType getOppositeTeam(TeamType team);
	static std::string toString(TeamType team);
};
