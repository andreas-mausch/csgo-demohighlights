#include <sstream>

#include "DemoParser.h"
#include "Entities.h"
#include "../gamestate/GameState.h"
#include "../parser/GameEventHandler.h"
#include "../sdk/demofiledump.h"

bool matchStarted = false;

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
		Player &attacker = gameState.findPlayerByUserId(getValue(message, descriptor, "attacker").val_short());
		Player &userid = gameState.findPlayerByUserId(getValue(message, descriptor, "userid").val_short());
		bool headshot = getValue(message, descriptor, "headshot").val_bool();
		userid.setAlive(false);

		gameEventHandler.playerDeath(userid, attacker, headshot);
		log.logVerbose("player_death: %s killed %s", attacker.toString().c_str(), userid.toString().c_str());
	}
}

void DemoParser::bombPlanted(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	Player &player = gameState.findPlayerByUserId(userId);
	gameState.updatePlayerPlanting(userId, false);
	gameState.setBombPosition(player.getPosition());

	gameEventHandler.bombPlanted();
	gameState.setBombPlantedTick(gameState.getTick());
}

void DemoParser::roundStart(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	gameState.setRoundStartedTick(gameState.getTick());
	gameState.setBombPlantedTick(-1);
	int roundTime = getValue(message, descriptor, "timelimit").val_long();
	gameState.setRoundTime(roundTime);
	log.logVerbose("roundStart: timelimit: %d; tick: %d", roundTime, gameState.getTick());

	std::vector<Player> &players = gameState.getPlayers();
	for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
	{
		player->setAlive(true);
		player->setPlanting(false, -1);
		player->setDefusing(false, -1, false);
		log.logVerbose("\t%s", player->toString().c_str());
	}

	gameEventHandler.roundStart();
}

void DemoParser::roundFreezeEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	log.logVerbose("roundFreezeEnd");
	gameEventHandler.roundFreezeEnd();
	gameState.setRoundStartedTick(gameState.getTick());
}

void DemoParser::roundEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	TeamType winner = Team::fromEngineInteger(getValue(message, descriptor, "winner").val_byte());
	log.logVerbose("roundEnd %s %d:%d (%d T's alive, %d CT's alive)", Team::toString(winner).c_str(), gameState.getRoundsWon(Terrorists), gameState.getRoundsWon(CounterTerrorists), gameState.getPlayersAlive(Terrorists), gameState.getPlayersAlive(CounterTerrorists));

	gameEventHandler.roundEnd(winner);
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

	playerConnectHandler.playerConnect(name, entityId, userId);
	gameEventHandler.playerConnect(name, entityId, userId);
}

void DemoParser::playerDisconnect(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	playerConnectHandler.playerDisconnect(userId);
	gameEventHandler.playerDisconnect(userId);
}

void DemoParser::announcePhaseEnd(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	log.logVerbose("announcePhaseEnd");
	gameEventHandler.announcePhaseEnd();
}

void DemoParser::weaponFire(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	std::string weapon = getValue(message, descriptor, "weapon").val_string();

	Player &player = gameState.findPlayerByUserId(userId);

	if (weapon == "smokegrenade")
	{
		gameEventHandler.grenadeThrown(SMOKEGRENADE, player);
	}
	else if (weapon == "flashbang")
	{
		gameEventHandler.grenadeThrown(FLASHBANG, player);
	}
	else if (weapon == "hegrenade")
	{
		gameEventHandler.grenadeThrown(HE_GRENADE, player);
	}
	else if (weapon == "decoy")
	{
		gameEventHandler.grenadeThrown(DECOY, player);
	}
	else if (weapon == "molotov" || weapon == "incgrenade")
	{
		gameEventHandler.grenadeThrown(MOLOTOV, player);
	}
}

void DemoParser::grenadeDetonate(Weapon type, CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	float x = getValue(message, descriptor, "x").val_float();
	float y = getValue(message, descriptor, "y").val_float();
	float z = getValue(message, descriptor, "z").val_float();

	Player &player = gameState.findPlayerByUserId(userId);
	Vector position(x, y, z);

	gameEventHandler.grenadeDetonate(type, player, position);
}

void DemoParser::bombBeginPlant(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	gameState.updatePlayerPlanting(userId, true);
}

void DemoParser::bombAbortPlant(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	gameState.updatePlayerPlanting(userId, false);
}

void DemoParser::bombBeginDefuse(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	bool kit = getValue(message, descriptor, "haskit").val_bool();
	gameState.updatePlayerDefusing(userId, true, kit);
}

void DemoParser::bombAbortDefuse(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	gameState.updatePlayerDefusing(userId, false, false);
}

void DemoParser::bombDefused(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor)
{
	int userId = getValue(message, descriptor, "userid").val_short();
	gameState.updatePlayerDefusing(userId, false, false);
	gameState.setBombPlantedTick(-1);
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
	else if (descriptor.name() == "weapon_fire")
	{
		weaponFire(message, descriptor);
	}
	else if (descriptor.name() == "hegrenade_detonate")
	{
		grenadeDetonate(HE_GRENADE, message, descriptor);
	}
	else if (descriptor.name() == "flashbang_detonate")
	{
		grenadeDetonate(FLASHBANG, message, descriptor);
	}
	else if (descriptor.name() == "smokegrenade_detonate")
	{
		grenadeDetonate(SMOKEGRENADE, message, descriptor);
	}
	else if (descriptor.name() == "molotov_detonate")
	{
		grenadeDetonate(MOLOTOV, message, descriptor);
	}
	else if (descriptor.name() == "decoy_detonate")
	{
		grenadeDetonate(DECOY, message, descriptor);
	}
	else if (descriptor.name() == "bomb_beginplant")
	{
		bombBeginPlant(message, descriptor);
	}
	else if (descriptor.name() == "bomb_abortplant")
	{
		bombAbortPlant(message, descriptor);
	}
	else if (descriptor.name() == "bomb_planted")
	{
		bombPlanted(message, descriptor);
	}
	else if (descriptor.name() == "bomb_begindefuse")
	{
		bombBeginDefuse(message, descriptor);
	}
	else if (descriptor.name() == "bomb_abortdefuse")
	{
		bombAbortDefuse(message, descriptor);
	}
	else if (descriptor.name() == "bomb_defused")
	{
		bombDefused(message, descriptor);
	}
}

void DemoParser::gameEventList(CSVCMsg_GameEventList &message)
{
	log.logVerbose("gameEventList");
	gameState.setGameEvents(message);
}
