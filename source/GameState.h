#pragma once

#include "protobuf/generated/netmessages_public.pb.h"

class MemoryStream;

class GameState
{
private:
	int tick;
	int positionInStream;

	CSVCMsg_GameEventList gameEvents;

public:
	GameState(int tick, int positionInStream);
	~GameState();

	const CSVCMsg_GameEventList::descriptor_t& getGameEvent(int eventId);

	void setTick(int tick);
	void setPositionInStream(int positionInStream);
	void setGameEvents(CSVCMsg_GameEventList &message);
};
