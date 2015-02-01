#include "Player.h"
#include "GameState.h"

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

Team Player::getTeam()
{
	return team;
}

void Player::setTeam(Team team)
{
	this->team = team;
}

int Player::getEntityId(GameState &gameState)
{
	int nIndex = 0;
	for (std::vector<Player>::iterator i = gameState.getPlayers().begin(); i != gameState.getPlayers().end(); i++)
	{
		if (i->getUserId() == userId)
		{
			return nIndex + 1;
		}
		nIndex++;
	}

	throw std::bad_exception("getEntityId()");
}
