#include <windows.h>
#include <commctrl.h>

#include "Demo.h"
#include "DemoviewerDialog.h"
#include "GameStateControl.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../../resources/resource.h"

DemoviewerDialog::DemoviewerDialog()
: Dialog(NULL, IDD_DEMOVIEWER), demo(NULL)
{
}

DemoviewerDialog::~DemoviewerDialog()
{
}

void DemoviewerDialog::setIcon()
{
	HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(dialog, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
	SendMessage(dialog, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
}

void DemoviewerDialog::setDemo(Demo &demo)
{
	this->demo = &demo;
	setGameState(&demo.getCurrentGameState());
}

void DemoviewerDialog::onPaint()
{
}

void DemoviewerDialog::onSize(int width, int height)
{
	int gameStateControlWidth = width;
	int maxHeight = height - 30;

	if (maxHeight < width)
	{
		gameStateControlWidth = maxHeight;
	}

	gameStateControlWidth -= 20;
	SetWindowPos(GetDlgItem(dialog, IDC_GAMESTATE), NULL, 0, 0, gameStateControlWidth, gameStateControlWidth, SWP_NOMOVE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(dialog, IDC_POSITION), NULL, 0, height - 30, width, 24, SWP_NOZORDER);
}

void DemoviewerDialog::setGameState(GameState *gameState)
{
	SendDlgItemMessage(dialog, IDC_GAMESTATE, WM_GAMESTATECONTROL, GAMESTATECONTROL_SET, reinterpret_cast<LPARAM>(gameState));

	SendDlgItemMessage(dialog, IDC_POSITION, TBM_SETRANGEMIN, FALSE, 0);
	SendDlgItemMessage(dialog, IDC_POSITION, TBM_SETRANGEMAX, FALSE, demo ? demo->getMaximumContinuousTick() : 0);
	SendDlgItemMessage(dialog, IDC_POSITION, TBM_SETPOS, TRUE, demo ? demo->getCurrentGameState().getContinuousTick() : 0);
}

void DemoviewerDialog::onScroll(int scrollType)
{
	int continuousTick = SendDlgItemMessage(dialog, IDC_POSITION, TBM_GETPOS, 0, 0);

	if (demo)
	{
		switch (scrollType)
		{
			case TB_PAGEUP:
			{
				demo->setPositionToPreviousRound();
			} break;
			case TB_PAGEDOWN:
			{
				demo->setPositionToNextRound();
			} break;
			default:
			{
				demo->setPosition(continuousTick);
			} break;
		}

		setGameState(&demo->getCurrentGameState());
	}
}

bool DemoviewerDialog::handleMessages()
{
	MSG message;
	BOOL ret = GetMessage(&message, 0, 0, 0);

	if (ret == -1)
	{
		return false;
	}
	else if (ret == 0)
	{
		// WM_QUIT
		return false;
	}
	else
	{
		if (!IsDialogMessage(dialog, &message))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	return true;
}

INT_PTR DemoviewerDialog::callback(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch (message)
		{
			case WM_INITDIALOG:
			{
				ShowWindow(dialog, SW_SHOW);
				setIcon();
			} break;
			case WM_PAINT:
			{
				onPaint();
			} break;
			case WM_SIZE:
			{
				onSize(LOWORD(lParam), HIWORD(lParam));
			} break;
			case WM_HSCROLL:
			{
				onScroll(LOWORD(wParam));
			} break;
			case WM_CLOSE:
			{
				close();
			} break;
		}
	}
	catch (const std::bad_exception &e)
	{
		MessageBoxA(0, e.what(), 0, 0);
	}

	return 0;
}
