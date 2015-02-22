#include "Team.h"

Team::Team(int entityId)
: entityId(entityId), score(0), type(UnknownTeam)
{
}

Team::~Team()
{
}

int Team::getScore()
{
	return score;
}

void Team::setScore(int score)
{
	this->score = score;
}

const std::string &Team::getName()
{
	return name;
}

int Team::getEntityId()
{
	return entityId;
}

void Team::setName(const std::string &name)
{
	this->name = name;
}

TeamType Team::getType()
{
	return type;
}

void Team::setType(TeamType type)
{
	this->type = type;
}

TeamType Team::fromEngineInteger(int i)
{
	switch (i)
	{
	case 2:
		return Terrorists;
	case 3:
		return CounterTerrorists;
	}

	return UnknownTeam;
}

TeamType Team::getOppositeTeam(TeamType type)
{
	switch (type)
	{
	case Terrorists:
		return CounterTerrorists;
	case CounterTerrorists:
		return Terrorists;
	}

	throw std::bad_exception("no opposite team");
}

std::string Team::toString(TeamType type)
{
	switch (type)
	{
	case Terrorists:
		return "Terrorists";
	case CounterTerrorists:
		return "Counter-Terrorists";
	case Spectator:
		return "Spectator";
	case UnknownTeam:
	default:
		return "Unknown";
	}
}
