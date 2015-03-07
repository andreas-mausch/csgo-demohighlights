#include "Demo.h"
#include "FilterHandler.h"

Demo::Demo(MemoryStreamBuffer &buffer)
: demo(buffer), maximumContinousTick(-1), currentGameState(0, 0)
{
}

Demo::~Demo()
{
}

void Demo::load()
{
	GameState gameState(0, demo.tellg());
	Log log(std::cout, false);
	FilterHandler filterHandler(gameState, log);
	DemoParser demoParser(gameState, log, filterHandler);
	demoParser.parseHeader(demo);
	gameStates.push_back(new GameState(gameState));

	int lastTickSaved = -1;
	while (demoParser.parseNextTick(demo))
	{
		if (abs(gameState.getTick() - lastTickSaved) > 1000)
		{
			GameState *s = new GameState(gameState);
			gameStates.push_back(s);
			lastTickSaved = gameState.getTick();
		}
	}
	maximumContinousTick = gameState.getContinuousTick();
	currentGameState = gameStates[0];
}

void Demo::setPosition(int continuousTick)
{
}

GameState &Demo::getCurrentGameState()
{
	return currentGameState;
}

GameState &Demo::findNearestGameState(int continuousTick)
{
	GameState *best = NULL;
	int bestTick = -1;

	for (PointerVector<GameState>::iterator s = gameStates.begin(); s != gameStates.end(); s++)
	{
		if ((*s)->getContinuousTick() <= continuousTick && (*s)->getContinuousTick() > bestTick)
		{
			bestTick = (*s)->getContinuousTick();
			best = *s;
		}
	}

	return *best;
}
