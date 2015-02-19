#include <windows.h>
#include <commctrl.h>

#include "GameStateControl.h"
#include "ImageDecoder.h"
#include "RenderBitmap.h"
#include "../../resources/resource.h"

GameStateControl::GameStateControl(HWND window)
: window(window)
{
	ImageDecoder imageDecoder;
	dust2 = imageDecoder.loadImageFromResource(MAKEINTRESOURCE(IDB_DE_DUST2), L"PNG");
}

GameStateControl::~GameStateControl()
{
	DeleteObject(dust2);
}

void GameStateControl::registerControl()
{
	WNDCLASSEX windowClass;
	memset(&windowClass, 0, sizeof(WNDCLASSEX));

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = L"GameStateControl";
	windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	windowClass.style = 0;
	windowClass.lpfnWndProc = callback;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.cbWndExtra = sizeof(GameStateControl *);
	RegisterClassEx(&windowClass);
}

void GameStateControl::onCreate()
{
}

void GameStateControl::onPaint()
{
	RECT clientRect;
	GetClientRect(window, &clientRect);

	PAINTSTRUCT paint;
	HDC deviceContext = BeginPaint(window, &paint);
	renderBitmap(paint.hdc, dust2, 0, 0);
	EndPaint(window, &paint);
}

LRESULT CALLBACK GameStateControl::callback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	GameStateControl *control = reinterpret_cast<GameStateControl *>(GetWindowLongPtr(window, 0));

	switch (message)
	{
		case WM_NCCREATE:
		{
			control = new GameStateControl(window);
			SetWindowLongPtr(window, 0, reinterpret_cast<LONG>(control));
		} break;
		case WM_NCDESTROY:
		{
			delete control;
		} break;
		case WM_CREATE:
		{
			control->onCreate();
			return 0;
		} break;
		case WM_PAINT:
		{
			control->onPaint();
			return 0;
		} break;
		case WM_GAMESTATECONTROL:
		{
			switch (wParam)
			{
			}
		}
	}

	return DefWindowProc(window, message, wParam, lParam);
}
