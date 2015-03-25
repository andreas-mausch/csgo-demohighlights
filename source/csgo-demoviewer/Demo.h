#pragma once

#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/utils/PointerVector.h"
#include "../csgo-demolibrary/streams/MemoryStream.h"

class MemoryStream;

class Demo
{
private:
	MemoryStream demo;
	PointerVector<GameState> gameStates;
	PointerVector<GameState> roundStarts;
	GameState currentGameState;
	int maximumContinousTick;

	void load();
	GameState &findNearestGameState(int continuousTick);

public:
	Demo(MemoryStreamBuffer &buffer);
	~Demo();

	void setPosition(int continuousTick);
	GameState &getCurrentGameState();
	int getMaximumContinuousTick();
	void addRoundStart(const GameState &gameState);
};
