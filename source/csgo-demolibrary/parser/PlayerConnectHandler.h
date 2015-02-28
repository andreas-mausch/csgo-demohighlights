#pragma once

#include "../../csgo-demolibrary/parser/GameEventHandler.h"

class GameState;
class Log;

class PlayerConnectHandler : public GameEventHandler
{
private:
	GameState &gameState;
	Log &log;

public:
	PlayerConnectHandler(GameState &gameState, Log &log);
	virtual ~PlayerConnectHandler();

	virtual void playerConnect(const std::string &name, int entityId, int userId);
	virtual void playerDisconnect(int userId);
};
