#include <sstream>

#include "Player.h"
#include "GameState.h"

Player::Player(int entityId, int userId, const std::string &name)
: entityId(entityId), userId(userId), name(name), team(UnknownTeam), alive(false), position(-1.0f, -1.0f, -1.0f), observer(false), health(-1), eyeAngle(-1.0f, -1.0f, -1.0f), planting(false), defusing(false)
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

void Player::setName(const std::string &name)
{
	this->name = name;
}

TeamType Player::getTeam()
{
	return team;
}

void Player::setTeam(TeamType team)
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

Vector Player::getEyeAngle()
{
	return eyeAngle;
}

void Player::setEyeAngle(Vector angle)
{
	eyeAngle = angle;
}

bool Player::isPlanting()
{
	return planting;
}

int Player::getPlantingTick()
{
	return plantingTick;
}

void Player::setPlanting(bool planting, int plantingTick)
{
	this->planting = planting;
	this->plantingTick = plantingTick;
}

bool Player::isDefusing()
{
	return defusing;
}

int Player::getDefusingTick()
{
	return defusingTick;
}

bool Player::hasKit()
{
	return kit;
}

void Player::setDefusing(bool defusing, int defusingTick, bool kit)
{
	this->defusing = defusing;
	this->defusingTick = defusingTick;
	this->kit = kit;
}

std::string Player::toString()
{
	std::stringstream output;
	output << getName() << " (" << Team::toString(getTeam()) << ")";
	return output.str();
}
