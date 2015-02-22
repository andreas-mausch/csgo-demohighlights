#pragma once

#define WM_GAMESTATECONTROL (WM_USER+1)

enum GameStateControlMessage
{
	GAMESTATECONTROL_SET
};

class GameState;
class Player;

class GameStateControl
{
private:
	HWND window;
	HBITMAP dust2;

	HDC backbuffer;
	HBITMAP backbufferBitmap;

	GameState *gameState;

	void renderMapBackground();
	void renderGeneralInfo();
	void renderPlayers();
	void renderPlayer(Player &player);
	void renderPercentagePie(int x, int y, int radius, int percentage);

	void deleteBackbuffer();
	void createBackbuffer();
	void paintBackbuffer();

	void onCreate();
	void onPaint();
	void onSize(int width, int height);

	static LRESULT CALLBACK callback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

public:
	GameStateControl(HWND window);
	~GameStateControl();

	static void registerControl();

};
