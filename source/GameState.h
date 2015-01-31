#pragma once

class MemoryStream;

class GameState
{
private:
	int tick;
	int positionInStream;

public:
	GameState(int tick, int positionInStream);
	~GameState();

	bool parseNextTick(MemoryStream &demo);
};
