#include <windows.h>
#include <commctrl.h>

#include "DemoviewerDialog.h"
#include "GameStateControl.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../../resources/resource.h"

DemoviewerDialog::DemoviewerDialog()
: Dialog(NULL, IDD_DEMOVIEWER)
{
}

DemoviewerDialog::~DemoviewerDialog()
{
}

void DemoviewerDialog::onPaint()
{
}

void DemoviewerDialog::onSize(int width, int height)
{
	int gameStateControlWidth = width;

	if (height < width)
	{
		gameStateControlWidth = height;
	}

	gameStateControlWidth -= 20;
	SetWindowPos(GetDlgItem(dialog, IDC_GAMESTATE), NULL, 0, 0, gameStateControlWidth, gameStateControlWidth, SWP_NOMOVE | SWP_NOZORDER);
}

void DemoviewerDialog::setGameState(GameState &gameState)
{
	SendDlgItemMessage(dialog, IDC_GAMESTATE, WM_GAMESTATECONTROL, GAMESTATECONTROL_SET, reinterpret_cast<LPARAM>(&gameState));

	SendDlgItemMessage(dialog, IDC_POSITION, TBM_SETRANGEMIN, FALSE, 0);
	SendDlgItemMessage(dialog, IDC_POSITION, TBM_SETRANGEMAX, FALSE, gameState.getHeader().playbackTicks);
	SendDlgItemMessage(dialog, IDC_POSITION, TBM_SETPOS, TRUE, gameState.getTick());
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
	switch (message)
	{
		case WM_INITDIALOG:
		{
			ShowWindow(dialog, SW_SHOW);
		} break;
		case WM_PAINT:
		{
			onPaint();
		} break;
		case WM_SIZE:
		{
			onSize(LOWORD(lParam), HIWORD(lParam));
		} break;
		case WM_CLOSE:
		{
			close();
		} break;
	}

	return 0;
}
