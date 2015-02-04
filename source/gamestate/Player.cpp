#include "Player.h"
#include "GameState.h"

Player::Player(int entityId, int userId, const std::string &name)
	: entityId(entityId), userId(userId), name(name), team(UnknownTeam), alive(false)
{
}

Player::~Player()
{
}

int Player::getUserId()
{
	return userId;
}

const std::string &Player::getName()
{
	return name;
}

Team Player::getTeam()
{
	return team;
}

void Player::setTeam(Team team)
{
	this->team = team;
}

bool Player::isAlive()
{
	return alive;
}

void Player::setAlive(bool alive)
{
	this->alive = alive;
}

int Player::getEntityId()
{
	return entityId;
}
