#include <sstream>

#include "DemoParser.h"
#include "Entities.h"
#include "../gamestate/GameState.h"
#include "../parser/GameEventHandler.h"
#include "../sdk/demofiledump.h"

int roundStartTick = -1;
bool matchStarted = false;

std::string toString(Player &player)
{
	std::stringstream output;
	output << player.getName() << " (" << toString(player.getTeam()) << ")";
	return output.str();
}

const CSVCMsg_GameEvent::key_t& getValue(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor, const std::string &keyName)
{
	for (int i = 0; i < descriptor.keys().size(); i++)
	{
		const CSVCMsg_GameEventList::key_t& key = descriptor.keys(i);
		if (key.name() == keyName)
		{
			return message.keys(i);
		}
	}

	throw std::bad_exception("key not found");
}

void DemoParser::playerDeath(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	if (matchStarted)
	{
		int tickDif = gameState.getTick() - roundStartTick;
		Player &attacker = gameState.findPlayerByUserId(getValue(message, descriptor, "attacker").val_short());
		Player &userid = gameState.findPlayerByUserId(getValue(message, descriptor, "userid").val_short());
		bool headshot = getValue(message, descriptor, "headshot").val_bool();
		userid.setAlive(false);

		gameEventHandler.playerDeath(userid, attacker, headshot);
		log.logVerbose("player_death: %s killed %s", toString(attacker).c_str(), toString(userid).c_str());
	}
}

void DemoParser::bombPlanted(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	gameEventHandler.bombPlanted();
}

void DemoParser::roundStart(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	log.logVerbose("roundStart: timelimit: %d; tick: %d", getValue(message, descriptor, "timelimit").val_long(), gameState.getTick());
	gameEventHandler.roundStart();
}

void DemoParser::roundFreezeEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	log.logVerbose("roundFreezeEnd");
	gameEventHandler.roundFreezeEnd();

	roundStartTick = gameState.getTick();

	std::vector<Player> &players = gameState.getPlayers();
	for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
	{
		int entityId = player->getEntityId();
		EntityEntry *entity = FindEntity(entityId);
		Team team = UnknownTeam;

		if (entity)
		{
			PropEntry *prop = entity->FindProp("m_iTeamNum");

			if (prop)
			{
				int teamInt = prop->m_pPropValue->m_value.m_int;
				team = fromEngineInteger(teamInt);
			}
		}

		player->setTeam(team);
		player->setAlive(true);
		log.logVerbose("\t%s", toString(*player).c_str());
	}
}

void DemoParser::roundEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	Team winner = fromEngineInteger(getValue(message, descriptor, "winner").val_byte());
	log.logVerbose("roundEnd %s %d:%d (%d T's alive, %d CT's alive)", toString(winner).c_str(), gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists), gameState.getPlayersAlive(Terrorists), gameState.getPlayersAlive(CounterTerrorists));

	gameEventHandler.roundEnd(winner);

	if (winner == Terrorists || winner == CounterTerrorists)
	{
		gameState.addWonRound(winner);
	}
}

void DemoParser::roundOfficiallyEnded(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	log.logVerbose("roundOfficiallyEnded");
	gameEventHandler.roundOfficiallyEnded();
}

void DemoParser::playerConnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	std::string name = getValue(message, descriptor, "name").val_string();
	int entityId = getValue(message, descriptor, "index").val_byte() + 1;
	int userId = getValue(message, descriptor, "userid").val_short();

	gameEventHandler.playerConnect(name, entityId, userId);
}

void DemoParser::playerDisconnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	gameEventHandler.playerDisconnect(userId);
}

void DemoParser::announcePhaseEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	log.logVerbose("announcePhaseEnd");
	gameEventHandler.announcePhaseEnd();
	gameState.switchTeams();
}

void DemoParser::gameEvent(CSVCMsg_GameEvent &message)
{
	const CSVCMsg_GameEventList::descriptor_t& descriptor = gameState.getGameEvent(message.eventid());
	log.logVerbose("gameEvent: %s" , descriptor.name().c_str());

	if (descriptor.name() == "player_death")
	{
		playerDeath(message, descriptor);
	}
	else if (descriptor.name() == "bomb_planted")
	{
		bombPlanted(message, descriptor);
	}
	else if (descriptor.name() == "round_start")
	{
		roundStart(message, descriptor);
	}
	else if (descriptor.name() == "round_freeze_end")
	{
		roundFreezeEnd(message, descriptor);
	}
	else if (descriptor.name() == "round_end")
	{
		roundEnd(message, descriptor);
	}
	else if (descriptor.name() == "round_officially_ended")
	{
		roundOfficiallyEnded(message, descriptor);
	}
	else if (descriptor.name() == "player_connect")
	{
		playerConnect(message, descriptor);
	}
	else if (descriptor.name() == "player_disconnect")
	{
		playerDisconnect(message, descriptor);
	}
	else if (descriptor.name() == "announce_phase_end")
	{
		announcePhaseEnd(message, descriptor);
	}
	else if (descriptor.name() == "round_announce_match_start")
	{
		matchStarted = true;
	}
}

void DemoParser::gameEventList(CSVCMsg_GameEventList &message)
{
	log.logVerbose("gameEventList");
	gameState.setGameEvents(message);
}
