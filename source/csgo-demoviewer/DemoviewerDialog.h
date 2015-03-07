#pragma once

#include "Dialog.h"

class Demo;
class GameState;

class DemoviewerDialog : public Dialog
{
private:
	Demo *demo;

	void onPaint();
	void onSize(int width, int height);
	void onScroll();

	void setGameState(GameState *gameState);

public:
	DemoviewerDialog();
	~DemoviewerDialog();

	virtual INT_PTR callback(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
	bool handleMessages();

	void setDemo(Demo &demo);

};
