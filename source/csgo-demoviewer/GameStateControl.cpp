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

COLORREF tColor = RGB(253, 205, 59);
COLORREF ctColor = RGB(90, 160, 222);
HBRUSH tBrush, ctBrush;

GameStateControl::GameStateControl(HWND window)
: window(window), gameState(NULL)
{
	ImageDecoder imageDecoder;
	dust2 = imageDecoder.loadImageFromResource(MAKEINTRESOURCE(IDB_DE_DUST2), L"PNG");
	tBrush = CreateSolidBrush(tColor);
	ctBrush = CreateSolidBrush(ctColor);
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

Vector worldToScreen(Vector position)
{
	return Vector((position.x + 2387.0f) / 5.54f, (3314.08f - position.y) / 5.54f, 0.0f);
}

void drawPosition(HDC deviceContext, Vector position, const std::string &name)
{
	Vector screen = worldToScreen(position);
	int size = 4;
	Ellipse(deviceContext, screen.x - size, screen.y - size, screen.x + size, screen.y + size);
	RECT rect;
	rect.left = screen.x + 10;
	rect.top = screen.y - 7;
	DrawTextA(deviceContext, name.c_str(), name.length(), &rect, DT_CALCRECT);
	DrawTextA(deviceContext, name.c_str(), name.length(), &rect, 0);
}

void GameStateControl::paintBackbuffer()
{
	RECT clientRect;
	GetClientRect(window, &clientRect);

	FillRect(backbuffer, &clientRect, ctBrush);
	renderBitmap(backbuffer, dust2, 0, 0);

					int y = 10;
					std::string text = formatString("players: %d; tick: %d", gameState->getPlayers().size(), gameState->getTick());
					TextOutA(backbuffer, 10, y, text.c_str(), text.length());

					std::vector<Player> &players = gameState->getPlayers();
					for (std::vector<Player>::iterator player = players.begin(); player != players.end(); player++)
					{
						DemofileVector position; position.x = 0.0f; position.y = 0.0f; position.z = 0.0f;
						float z = 0.0f;
						int observerMode = 0;
						int entityId = player->getEntityId();
						EntityEntry *entity = FindEntity(entityId);

						if (entity)
						{
							PropEntry *prop = entity->FindProp("m_vecOrigin");

							if (prop)
							{
								position = prop->m_pPropValue->m_value.m_vector;
							}
							prop = entity->FindProp("m_vecOrigin[2]");

							if (prop)
							{
								z = prop->m_pPropValue->m_value.m_float;
							}
							prop = entity->FindProp("m_iObserverMode");

							if (prop)
							{
								observerMode = prop->m_pPropValue->m_value.m_int;
							}
						}

						y += 20;
						std::string text = formatString("player: %s, %.2f, %.2f, %.2f", player->getName().c_str(), position.x, position.y, z);
						//TextOutA(backbuffer, 10, y, text.c_str(), text.length());
						if (observerMode == 0)
						{
							SetTextColor(backbuffer, player->isAlive() ? (player->getTeam() == Terrorists ? tColor : ctColor) : RGB(200, 200, 200));
							SetBkMode(backbuffer, TRANSPARENT);
							SelectObject(backbuffer, player->getTeam() == Terrorists ? tBrush : ctBrush);
							drawPosition(backbuffer, Vector(position.x, position.y, position.z), player->getName());
						}
					}
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
