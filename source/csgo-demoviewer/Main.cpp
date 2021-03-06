#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <string>

#include "Demo.h"
#include "DemoviewerDialog.h"
#include "GameStateControl.h"
#include "../csgo-demolibrary/parser/DemoParser.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/streams/MemoryStreamBuffer.h"
#include "../csgo-demolibrary/streams/MemoryStream.h"
#include "../csgo-demolibrary/utils/File.h"

DemoviewerDialog demoviewerDialog;

DWORD WINAPI myThread(void *p)
{
	try
	{
		const std::string filename = "demo.dem";
		std::string stringBuffer = readFile(filename);

		if (stringBuffer.length() == 0)
		{
			std::cout << "error reading file." << std::endl;
			return -1;
		}

		MemoryStreamBuffer demoBuffer(const_cast<char *>(stringBuffer.c_str()), stringBuffer.length());
		Demo demo(demoBuffer);
		demoviewerDialog.setDemo(demo);

		while (true)
		{
			Sleep(1000);
		}
	}
	catch (const std::bad_exception &e)
	{
		MessageBoxA(0, e.what(), 0, 0);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	CoInitialize(NULL);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	GameStateControl::registerControl();

	demoviewerDialog.open();

	CreateThread(NULL, 0, myThread, NULL, 0, NULL);

	while (demoviewerDialog.getHandle())
	{
		demoviewerDialog.handleMessages();
	}

	CoUninitialize();
	return 0;
}
