#include <windows.h>

#include "DemoviewerDialog.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	DemoviewerDialog demoviewerDialog;
	demoviewerDialog.open();

	while (demoviewerDialog.getHandle())
	{
		demoviewerDialog.handleMessages();
	}

	return 0;
}
