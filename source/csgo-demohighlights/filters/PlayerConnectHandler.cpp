#include "PlayerConnectHandler.h"
#include "../../csgo-demolibrary/gamestate/GameState.h"
#include "../../csgo-demolibrary/gamestate/Player.h"

PlayerConnectHandler::PlayerConnectHandler(GameState &gameState)
: gameState(gameState)
{
}

PlayerConnectHandler::~PlayerConnectHandler()
{
}

void PlayerConnectHandler::playerConnect(const std::string &name, int entityId, int userId)
{
	Player player(entityId, userId, name);
	gameState.updatePlayer(player);
}

void PlayerConnectHandler::playerDisconnect(int userId)
{
	gameState.disconnect(userId);
}
