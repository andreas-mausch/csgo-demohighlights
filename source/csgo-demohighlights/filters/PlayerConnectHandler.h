#pragma once

#include "../../csgo-demolibrary/parser/GameEventHandler.h"

class GameState;

class PlayerConnectHandler : public GameEventHandler
{
private:
	GameState &gameState;
public:
	PlayerConnectHandler(GameState &gameState);
	virtual ~PlayerConnectHandler();

	virtual void playerConnect(const std::string &name, int entityId, int userId);
	virtual void playerDisconnect(int userId);
};
