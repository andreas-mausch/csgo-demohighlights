#include "Demo.h"
#include "FilterHandlerRoundStart.h"

FilterHandlerRoundStart::FilterHandlerRoundStart(Demo &demo, GameState &gameState, Log &log)
: demo(demo), FilterHandler(gameState, log)
{
}

FilterHandlerRoundStart::~FilterHandlerRoundStart()
{
}

void FilterHandlerRoundStart::roundStart()
{
	demo.addRoundStart(gameState);
}
