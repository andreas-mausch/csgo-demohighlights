#include "GameState.h"

GameState::GameState(int tick, int positionInStream)
	: tick(tick), positionInStream(positionInStream)
{
}

GameState::~GameState()
{
}

void GameState::parseNextTick(MemoryStream &demo)
{
}
