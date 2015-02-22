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
	TeamType team;
	Vector position;
	int health;
	Vector eyeAngle;
	bool planting;
	int plantingTick;
	bool defusing;
	int defusingTick;
	bool kit;

public:
	Player(int entityId, int userId, const std::string &name);
	~Player();

	int getUserId();
	int getEntityId();

	Vector getPosition();
	void setPosition(Vector position);

	const std::string &getName();

	TeamType getTeam();
	void setTeam(TeamType team);

	bool isAlive();
	void setAlive(bool alive);

	bool isObserver();
	void setObserver(bool observer);

	int getHealth();
	void setHealth(int health);

	Vector getEyeAngle();
	void setEyeAngle(Vector angle);

	bool isPlanting();
	int getPlantingTick();
	void setPlanting(bool planting, int plantingTick);

	bool isDefusing();
	int getDefusingTick();
	bool hasKit();
	void setDefusing(bool defusing, int defusingTick, bool kit);
};
