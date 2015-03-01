#pragma once

#include "Dialog.h"

class GameState;

class DemoviewerDialog : public Dialog
{
private:

	void onPaint();
	void onSize(int width, int height);

public:
	DemoviewerDialog();
	~DemoviewerDialog();

	virtual INT_PTR callback(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
	bool handleMessages();

	void setGameState(GameState &gameState);

};
