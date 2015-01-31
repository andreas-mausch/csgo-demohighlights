#include "Player.h"

Player::Player(int userId, const std::string &name)
	: userId(userId), name(name)
{
}

Player::~Player()
{
}

const std::string &Player::getName()
{
	return name;
}
