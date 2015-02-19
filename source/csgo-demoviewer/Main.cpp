#include <windows.h>
#include <commctrl.h>

#include "DemoviewerDialog.h"
#include "GameStateControl.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	CoInitialize(NULL);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	GameStateControl::registerControl();

	DemoviewerDialog demoviewerDialog;
	demoviewerDialog.open();

	while (demoviewerDialog.getHandle())
	{
		demoviewerDialog.handleMessages();
	}

	CoUninitialize();
	return 0;
}
