#pragma once

#include <string>

#include "Team.h"
#include "../utils/Vector.h"

class Player
{
private:
	int entityId;
	int userId;
	std::string name;
	bool alive;
	bool observer;
	Team team;
	Vector position;

public:
	Player(int entityId, int userId, const std::string &name);
	~Player();

	int getUserId();
	int getEntityId();

	Vector getPosition();
	void setPosition(Vector position);

	const std::string &getName();

	Team getTeam();
	void setTeam(Team team);
	void switchTeam();

	bool isAlive();
	void setAlive(bool alive);

	bool isObserver();
	void setObserver(bool observer);
};
