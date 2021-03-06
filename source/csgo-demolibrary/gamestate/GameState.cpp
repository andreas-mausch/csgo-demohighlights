#include "GameState.h"

#include "../sdk/demofile.h"
#include "../streams/MemoryStream.h"
#include "../utils/StringFormat.h"

GameState::GameState(int tick, int positionInStream)
: tick(tick), continuousTick(0), positionInStream(positionInStream), s_nServerClassBits(0)
{
}

GameState::~GameState()
{
}

DemoHeader &GameState::getHeader()
{
	return header;
}

void GameState::setHeader(DemoHeader header)
{
	this->header = header;
}

int GameState::getTick()
{
	return tick;
}

void GameState::setTick(int tick)
{
	this->tick = tick;
}

int GameState::getContinuousTick()
{
	return continuousTick;
}

void GameState::setContinuousTick(int tick)
{
	this->continuousTick = tick;
}

int GameState::getPositionInStream()
{
	return positionInStream;
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

std::vector<Stringtable> &GameState::getStringTables()
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

Player *GameState::findPlayerByEntityIdIfExists(int entityId)
{
	for (std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (it->getEntityId() == entityId)
		{
			return &*it;
		}
	}

	return NULL;
}

Player *GameState::findPlayerByUserIdIfExists(int userId)
{
	for (std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
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
		existingPlayer->setName(player.getName());
	}
	else
	{
		players.push_back(player);
	}
}

void GameState::updatePlayerTeam(int entityId, TeamType team)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player)
	{
		player->setTeam(team);
	}
}

void GameState::updatePlayerPositionXY(int entityId, float x, float y)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player && player->isAlive())
	{
		Vector position = player->getPosition();
		position.x = x;
		position.y = y;
		player->setPosition(position);
	}
}

void GameState::updatePlayerPositionZ(int entityId, float z)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player && player->isAlive())
	{
		Vector position = player->getPosition();
		position.z = z;
		player->setPosition(position);
	}
}

void GameState::updatePlayerObserverMode(int entityId, bool observer)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player)
	{
		player->setObserver(observer);
	}
}

void GameState::updatePlayerHealth(int entityId, int health)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player)
	{
		player->setHealth(health);
	}
}

void GameState::updatePlayerEyeAngleX(int entityId, float x)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player)
	{
		Vector angle = player->getEyeAngle();
		angle.x = x;
		player->setEyeAngle(angle);
	}
}

void GameState::updatePlayerEyeAngleY(int entityId, float y)
{
	Player *player = findPlayerByEntityIdIfExists(entityId);

	if (player)
	{
		Vector angle = player->getEyeAngle();
		angle.y = y;
		player->setEyeAngle(angle);
	}
}

void GameState::updatePlayerPlanting(int userId, bool planting)
{
	findPlayerByUserId(userId).setPlanting(planting, tick);
}

void GameState::updatePlayerDefusing(int userId, bool defusing, bool kit)
{
	findPlayerByUserId(userId).setDefusing(defusing, tick, kit);
}

int GameState::getPlayersAlive(TeamType type)
{
	int playersAlive = 0;

	for(std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (it->getTeam() == type && it->isAlive())
		{
			playersAlive++;
		}
	}

	return playersAlive;
}

int GameState::getRoundsWon(TeamType type)
{
	return getTeam(type).getScore();
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

Team *GameState::getTeamIfExists(TeamType type)
{
	for(std::vector<Team>::iterator it = teams.begin(); it != teams.end(); ++it)
	{
		if (it->getType() == type)
		{
			return &*it;
		}
	}

	return NULL;
}

Team &GameState::getTeam(TeamType type)
{
	Team *team = getTeamIfExists(type);

	if (team == NULL)
	{
		throw std::bad_exception("team not found");
	}

	return *team;
}

Team &GameState::getTeamByEntityId(int entityId)
{
	for(std::vector<Team>::iterator it = teams.begin(); it != teams.end(); ++it)
	{
		if (it->getEntityId() == entityId)
		{
			return *it;
		}
	}

	throw std::bad_exception("team not found");
}

void GameState::addTeam(int entityId)
{
	teams.push_back(Team(entityId));
}

void GameState::updateTeamname(int entityId, const std::string &name)
{
	getTeamByEntityId(entityId).setName(name);
}

void GameState::updateTeamType(int entityId, TeamType type)
{
	getTeamByEntityId(entityId).setType(type);
}

void GameState::updateTeamScore(int entityId, int score)
{
	getTeamByEntityId(entityId).setScore(score);
}

void GameState::setRoundTime(int seconds)
{
	round.roundTime = seconds;
}

void GameState::setRoundStartedTick(int tick)
{
	round.roundStartedTick = tick;
}

void GameState::setBombTimer(int seconds)
{
	round.bombTimer = seconds;
}

int GameState::getBombPlantedTick()
{
	return round.bombPlantedTick;
}

void GameState::setBombPlantedTick(int tick)
{
	round.bombPlantedTick = tick;
}

int GameState::getTicksPerSecond()
{
	return static_cast<int>(static_cast<float>(header.playbackTicks) / header.playbackTime);
}

int GameState::getRoundTimeLeft()
{
	return round.roundTime - (tick - round.roundStartedTick) / getTicksPerSecond();
}

int GameState::getBombTimeLeft()
{
	return round.bombTimer - (tick - round.bombPlantedTick) / getTicksPerSecond();
}

Vector GameState::getBombPosition()
{
	return round.bombPosition;
}

void GameState::setBombPosition(Vector position)
{
	round.bombPosition = position;
}

PointerVector<EntityEntry> &GameState::getEntities()
{
	return s_Entities;
}
