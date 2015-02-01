#pragma once

#include <string>

#include "Team.h"

class GameState;

class Player
{
private:
	int userId;
	std::string name;
	Team team;

	int getEntityId(GameState &gameState);

public:
	Player(int userId, const std::string &name);
	~Player();

	int getUserId();
	const std::string &getName();

	Team getTeam();
	void setTeam(Team team);
};
