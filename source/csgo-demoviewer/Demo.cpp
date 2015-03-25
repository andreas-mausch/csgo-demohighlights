#include "Demo.h"
#include "FilterHandler.h"
#include "FilterHandlerRoundStart.h"

Demo::Demo(MemoryStreamBuffer &buffer)
: demo(buffer), maximumContinousTick(-1), currentGameState(0, 0)
{
	load();
}

Demo::~Demo()
{
}

void Demo::load()
{
	GameState gameState(0, demo.tellg());
	Log log(std::cout, false);
	FilterHandlerRoundStart filterHandler(*this, gameState, log);
	DemoParser demoParser(gameState, log, filterHandler);
	demoParser.parseHeader(demo);
	gameStates.push_back(new GameState(gameState));

	int lastTickSaved = -1;
	while (demoParser.parseNextTick(demo))
	{
		if (abs(gameState.getContinuousTick() - lastTickSaved) > 1000)
		{
			GameState *s = new GameState(gameState);
			gameStates.push_back(s);
			lastTickSaved = gameState.getContinuousTick();
		}
	}
	maximumContinousTick = gameState.getContinuousTick();
	currentGameState = gameStates[0];
}

void Demo::setPosition(int continuousTick)
{
	if (currentGameState.getContinuousTick() < continuousTick &&
		(continuousTick - currentGameState.getContinuousTick()) < 500)
	{
	}
	else
	{
		GameState &originGameState = findNearestGameState(continuousTick);
		currentGameState = originGameState;
	}

	demo.seekg(currentGameState.getPositionInStream());
	Log log(std::cout, false);
	FilterHandler filterHandler(currentGameState, log);
	DemoParser demoParser(currentGameState, log, filterHandler);
	while (currentGameState.getContinuousTick() < continuousTick)
	{
		demoParser.parseNextTick(demo);
	}
}

void Demo::setPositionToNextRound()
{
	for (PointerVector<GameState>::iterator s = roundStarts.begin(); s != roundStarts.end(); s++)
	{
		if ((*s)->getContinuousTick() > currentGameState.getContinuousTick())
		{
			setPosition((*s)->getContinuousTick());
			return;
		}
	}
}

void Demo::setPositionToPreviousRound()
{
	GameState *previous = NULL;

	for (PointerVector<GameState>::iterator s = roundStarts.begin(); s != roundStarts.end(); s++)
	{
		if ((*s)->getContinuousTick() >= currentGameState.getContinuousTick())
		{
			if (previous)
			{
				setPosition(previous->getContinuousTick());
			}
			else
			{
				setPosition(0);
			}

			return;
		}

		previous = *s;
	}
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

void Demo::addRoundStart(const GameState &gameState)
{
	roundStarts.push_back(new GameState(gameState));
}

int Demo::getMaximumContinuousTick()
{
	return maximumContinousTick;
}
