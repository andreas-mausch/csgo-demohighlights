#include "Player.h"
#include "GameState.h"

Player::Player(int entityId, int userId, const std::string &name)
	: entityId(entityId), userId(userId), name(name), team(Unknown)
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

int Player::getEntityId()
{
	return entityId;
}
