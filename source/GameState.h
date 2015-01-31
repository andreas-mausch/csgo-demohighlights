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

	void parseNextTick(MemoryStream &demo);
};
