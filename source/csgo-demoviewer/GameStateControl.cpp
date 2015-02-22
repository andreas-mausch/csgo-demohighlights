#include <windows.h>
#include <commctrl.h>

#include "GameStateControl.h"
#include "ImageDecoder.h"
#include "RenderBitmap.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/parser/Entities.h"
#include "../csgo-demolibrary/utils/StringFormat.h"
#include "../csgo-demolibrary/utils/Vector.h"
#include "../../resources/resource.h"

GameStateControl::GameStateControl(HWND window)
: window(window), gameState(NULL), tColor(RGB(253, 205, 59)), ctColor(RGB(90, 160, 222))
{
	ImageDecoder imageDecoder;
	dust2 = imageDecoder.loadImageFromResource(MAKEINTRESOURCE(IDB_DE_DUST2), L"PNG");
	tBrush = CreateSolidBrush(tColor);
	ctBrush = CreateSolidBrush(ctColor);
}

GameStateControl::~GameStateControl()
{
	DeleteObject(ctBrush);
	DeleteObject(tBrush);
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

void GameStateControl::deleteBackbuffer()
{
	DeleteObject(backbufferBitmap);
	backbufferBitmap = NULL;

	DeleteDC(backbuffer);
	backbuffer = NULL;
}

void GameStateControl::createBackbuffer()
{
	deleteBackbuffer();

	RECT clientRect;
	GetClientRect(window, &clientRect);

	HDC deviceContext = GetDC(window);

	backbuffer = CreateCompatibleDC(deviceContext);
	backbufferBitmap = CreateCompatibleBitmap(deviceContext, clientRect.right, clientRect.bottom);
	SelectObject(backbuffer, backbufferBitmap);

	ReleaseDC(window, deviceContext);
}

Vector worldToScreen(Vector position, int width, int height)
{
	return Vector((position.x + 2387.0f) / 4407.0f * width, (3314.08f - position.y) / 4407.0f * height, 0.0f);
}

void GameStateControl::renderPercentagePie(int x, int y, int radius, int percentage)
{
	double PI = 3.1415926;
	double sa = 2.0 * PI * 0 / 100.0;
	double ea = 2.0 * PI * percentage / 100.0;
	int xsa = x + radius * sin(sa);
	int ysa = y + radius * cos(sa);
	int xea = x + radius * sin(ea);
	int yea = y + radius * cos(ea);
	Pie(backbuffer, x - radius, y - radius, x + radius, y + radius, xsa, ysa, xea, yea);
}

void GameStateControl::renderText(int x, int y, const std::string &text)
{
	RECT rect;
	rect.left = x;
	rect.top = y;
	SelectObject(backbuffer, GetStockObject(DEFAULT_GUI_FONT));
	DrawTextA(backbuffer, text.c_str(), text.length(), &rect, DT_CALCRECT);
	DrawTextA(backbuffer, text.c_str(), text.length(), &rect, 0);
}

void GameStateControl::renderPlayer(Player &player)
{
	RECT clientRect;
	GetClientRect(window, &clientRect);

	SelectObject(backbuffer, GetStockObject(BLACK_BRUSH));
	SelectObject(backbuffer, GetStockObject(BLACK_PEN));
	Vector screen = worldToScreen(player.getPosition(), clientRect.right, clientRect.bottom);

	int size = 5;
	if (player.isAlive())
	{
		Ellipse(backbuffer, screen.x - size, screen.y - size, screen.x + size, screen.y + size);
		SelectObject(backbuffer, player.getTeam() == Terrorists ? tBrush : ctBrush);

		if (player.getHealth() == 100)
		{
			Ellipse(backbuffer, screen.x - size, screen.y - size, screen.x + size, screen.y + size);
		}
		else if (player.getHealth() > 0)
		{
			renderPercentagePie(screen.x, screen.y, size, player.getHealth());
		}
	}
	else
	{
		MoveToEx(backbuffer, screen.x - size, screen.y - size, NULL);
		LineTo(backbuffer, screen.x + size, screen.y + size);
		MoveToEx(backbuffer, screen.x + size, screen.y - size, NULL);
		LineTo(backbuffer, screen.x - size, screen.y + size);
	}

	std::string text;
	if (player.isAlive())
	{
		text = formatString("%s (%d)", player.getName().c_str(), player.getHealth());
	}
	else
	{
		text = formatString("%s", player.getName().c_str());
	}

	renderText(screen.x + 10, screen.y - 7, text);
}

void GameStateControl::renderScore()
{
	std::string text = formatString("%d:%d", gameState->getRoundsWon(Terrorists), gameState->getRoundsWon(CounterTerrorists));
	RECT clientRect;
	GetClientRect(window, &clientRect);

	RECT rect;
	rect.left = 0;
	rect.top = 20;
	SelectObject(backbuffer, GetStockObject(DEFAULT_GUI_FONT));
	SetTextColor(backbuffer, RGB(255, 255, 255));
	SetBkMode(backbuffer, TRANSPARENT);
	DrawTextA(backbuffer, text.c_str(), text.length(), &rect, DT_CALCRECT);

	rect.left += (clientRect.right - rect.right) / 2;
	rect.right += (clientRect.right - rect.right) / 2;

	DrawTextA(backbuffer, text.c_str(), text.length(), &rect, 0);
}

void GameStateControl::renderMapBackground()
{
	RECT clientRect;
	GetClientRect(window, &clientRect);

	renderBitmapStretched(backbuffer, dust2, 0, 0, clientRect.right, clientRect.bottom);
}

void GameStateControl::renderGeneralInfo()
{
	RECT clientRect;
	GetClientRect(window, &clientRect);

	SetTextColor(backbuffer, RGB(255, 255, 255));
	SetBkMode(backbuffer, TRANSPARENT);
	std::string text = formatString("tick: %d", gameState->getTick());
	renderText(clientRect.right - 80, clientRect.bottom - 20, text);
}

void GameStateControl::renderPlayers()
{
	std::vector<Player> &players = gameState->getPlayers();
	for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
	{
		if (!(player->isObserver() && player->isAlive()))
		{
			SetTextColor(backbuffer, player->isAlive() ? (player->getTeam() == Terrorists ? tColor : ctColor) : RGB(200, 200, 200));
			SetBkMode(backbuffer, TRANSPARENT);
			renderPlayer(*player);
		}
	}
}

void GameStateControl::paintBackbuffer()
{
	renderMapBackground();
	renderGeneralInfo();
	renderPlayers();
	renderScore();
}

void GameStateControl::onCreate()
{
	createBackbuffer();
}

void GameStateControl::onPaint()
{
	if (!backbuffer)
	{
		createBackbuffer();
	}
	paintBackbuffer();

	RECT clientRect;
	GetClientRect(window, &clientRect);

	PAINTSTRUCT paint;
	BeginPaint(window, &paint);
	BitBlt(paint.hdc, 0, 0, clientRect.right, clientRect.bottom, backbuffer, 0, 0, SRCCOPY);
	EndPaint(window, &paint);
}

void GameStateControl::onSize(int width, int height)
{
	createBackbuffer();
	InvalidateRect(window, NULL, TRUE);
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
		case WM_SIZE:
		{
			control->onSize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		} break;
		case WM_GAMESTATECONTROL:
		{
			switch (wParam)
			{
				case GAMESTATECONTROL_SET:
				{
					control->gameState = reinterpret_cast<GameState *>(lParam);
					RedrawWindow(window, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN);
				} break;
			}
		} break;
	}

	return DefWindowProc(window, message, wParam, lParam);
}
