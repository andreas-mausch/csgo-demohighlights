#pragma once

struct Round
{
	Round() : roundTime(-1), bombTimer(45), roundStartedTick(-1), bombPosition(-1.0f, -1.0f, -1.0f), bombPlantedTick(-1) {}

	int roundTime;
	int bombTimer;
	int roundStartedTick;
	int bombPlantedTick;
	Vector bombPosition;
};
