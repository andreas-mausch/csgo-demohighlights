#include "PlayerConnectHandler.h"
#include "../gamestate/GameState.h"
#include "../gamestate/Player.h"
#include "../utils/Log.h"

PlayerConnectHandler::PlayerConnectHandler(GameState &gameState, Log &log)
: gameState(gameState), log(log)
{
}

PlayerConnectHandler::~PlayerConnectHandler()
{
}

void PlayerConnectHandler::playerConnect(const std::string &name, int entityId, int userId)
{
	log.logVerbose("playerConnect %s %d %d", name.c_str(), entityId, userId);
	Player player(entityId, userId, name);
	gameState.updatePlayer(player);
}

void PlayerConnectHandler::playerDisconnect(int userId)
{
	log.logVerbose("playerDisconnect %d", userId);
	gameState.disconnect(userId);
}
