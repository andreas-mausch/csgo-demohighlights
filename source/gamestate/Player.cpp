#include "Player.h"

Player::Player(int userId, const std::string &name)
	: userId(userId), name(name)
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
