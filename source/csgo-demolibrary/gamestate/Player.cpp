#include "Player.h"
#include "GameState.h"

Player::Player(int entityId, int userId, const std::string &name)
: entityId(entityId), userId(userId), name(name), team(UnknownTeam), alive(false), position(-1.0f, -1.0f, -1.0f), observer(false), health(-1)
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

void Player::switchTeam()
{
	if (team == Terrorists || team == CounterTerrorists)
	{
		team = getOppositeTeam(team);
	}
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

Vector Player::getPosition()
{
	return position;
}

void Player::setPosition(Vector position)
{
	this->position = position;
}

bool Player::isObserver()
{
	return observer;
}

void Player::setObserver(bool observer)
{
	this->observer = observer;
}

int Player::getHealth()
{
	return health;
}

void Player::setHealth(int health)
{
	this->health = health;
}
