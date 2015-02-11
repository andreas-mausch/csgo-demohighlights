#pragma once

#include <string>

#include "Team.h"

class Player
{
private:
	int entityId;
	int userId;
	std::string name;
	bool alive;
	Team team;

public:
	Player(int entityId, int userId, const std::string &name);
	~Player();

	int getUserId();
	int getEntityId();

	const std::string &getName();

	Team getTeam();
	void setTeam(Team team);
	void switchTeam();

	bool isAlive();
	void setAlive(bool alive);
};
