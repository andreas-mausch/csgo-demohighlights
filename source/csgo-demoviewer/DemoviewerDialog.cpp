#include <windows.h>

#include "DemoviewerDialog.h"
#include "GameStateControl.h"
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

void DemoviewerDialog::renderGameState(GameState &gameState)
{
	SendDlgItemMessage(dialog, IDC_GAMESTATE, WM_GAMESTATECONTROL, GAMESTATECONTROL_SET, reinterpret_cast<LPARAM>(&gameState));
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
		case WM_CLOSE:
		{
			close();
		} break;
	}

	return 0;
}
