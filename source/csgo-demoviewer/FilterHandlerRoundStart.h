#pragma once

#include "FilterHandler.h"

class Demo;

class FilterHandlerRoundStart : public FilterHandler
{
private:
	Demo &demo;

public:
	FilterHandlerRoundStart(Demo &demo, GameState &gameState, Log &log);
	virtual ~FilterHandlerRoundStart();

	virtual void roundStart();
};
