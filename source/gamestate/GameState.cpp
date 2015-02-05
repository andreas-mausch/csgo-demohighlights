#include "GameState.h"
#include "../Main.h"

#include "../sdk/demofile.h"
#include "../streams/MemoryStream.h"
#include "../utils/StringFormat.h"

GameState::GameState(int tick, int positionInStream)
	: tick(tick), positionInStream(positionInStream), tRoundsWon(0), ctRoundsWon(0)
{
}

GameState::~GameState()
{
}

int GameState::getTick()
{
	return tick;
}

void GameState::setTick(int tick)
{
	this->tick = tick;
}

void GameState::setPositionInStream(int positionInStream)
{
	this->positionInStream = positionInStream;
}

void GameState::setGameEvents(CSVCMsg_GameEventList &message)
{
	this->gameEvents = message;
}

const CSVCMsg_GameEventList::descriptor_t &GameState::getGameEvent(int eventId)
{
	for (int i = 0; i < gameEvents.descriptors().size(); i++ )
	{
		const CSVCMsg_GameEventList::descriptor_t& Descriptor = gameEvents.descriptors( i );

		if ( Descriptor.eventid() == eventId )
		{
			return Descriptor;
		}
	}

	throw std::bad_exception("game event not found");
}

std::vector<StringTableData_t> &GameState::getStringTables()
{
	return stringTables;
}

std::vector<Player> &GameState::getPlayers()
{
	return players;
}

Player &GameState::findPlayerByUserId(int userId)
{
	Player *player = findPlayerByUserIdIfExists(userId);

	if (!player)
	{
		throw std::bad_exception("player not found");
	}

	return *player;
}

Player *GameState::findPlayerByUserIdIfExists(int userId)
{
	for(std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (it->getUserId() == userId)
		{
			return &*it;
		}
	}

	return NULL;
}

void GameState::updatePlayer(Player &player)
{
	Player *existingPlayer = findPlayerByUserIdIfExists(player.getUserId());

	if (existingPlayer)
	{
		*existingPlayer = player;
	}
	else
	{
		players.push_back(player);
	}
}

int GameState::getPlayersAlive(Team team)
{
	int playersAlive = 0;

	for(std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (it->getTeam() == team && it->isAlive())
		{
			playersAlive++;
		}
	}

	return playersAlive;
}

int GameState::getRoundsWon(Team team)
{
	switch (team)
	{
	case Terrorists:
		return tRoundsWon;
	case CounterTerrorists:
		return ctRoundsWon;
	}

	throw std::bad_exception("invalid call getRoundsWon()");
}

void GameState::addWonRound(Team team)
{
	switch (team)
	{
	case Terrorists:
		tRoundsWon++; return;
	case CounterTerrorists:
		ctRoundsWon++; return;
	}
}

void GameState::disconnect(int userId)
{
	for(std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (it->getUserId() == userId)
		{
			players.erase(it);
			return;
		}
	}
}

void GameState::switchTeams()
{
	for(std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
	{
		it->switchTeam();
	}

	int tRoundsWon = this->tRoundsWon;
	int ctRoundsWon = this->ctRoundsWon;

	this->tRoundsWon = ctRoundsWon;
	this->ctRoundsWon = tRoundsWon;
}
