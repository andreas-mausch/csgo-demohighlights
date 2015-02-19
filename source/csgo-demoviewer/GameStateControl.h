#pragma once

#define WM_GAMESTATECONTROL (WM_USER+1)

enum GameStateControlMessage
{
	GAMESTATECONTROL_SET
};

class GameStateControl
{
private:
	HWND window;
	HBITMAP dust2;

	void onCreate();
	void onPaint();

	static LRESULT CALLBACK callback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

public:
	GameStateControl(HWND window);
	~GameStateControl();

	static void registerControl();

};
