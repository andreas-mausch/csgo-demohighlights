#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <string>

#include "DemoviewerDialog.h"
#include "FilterHandler.h"
#include "GameStateControl.h"
#include "../csgo-demolibrary/parser/DemoParser.h"
#include "../csgo-demolibrary/gamestate/GameState.h"
#include "../csgo-demolibrary/streams/MemoryStreamBuffer.h"
#include "../csgo-demolibrary/streams/MemoryStream.h"
#include "../csgo-demolibrary/utils/File.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	CoInitialize(NULL);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	GameStateControl::registerControl();

	const std::string filename = "demo.dem";
	std::string stringBuffer = readFile(filename);

	if (stringBuffer.length() == 0)
	{
		std::cout << "error reading file." << std::endl;
		return -1;
	}

	MemoryStreamBuffer demoBuffer(const_cast<char *>(stringBuffer.c_str()), stringBuffer.length());
	MemoryStream demo(demoBuffer);

	bool end = false;
	GameState gameState(0, demo.tellg());
	Log log(std::cout, false);
	FilterHandler filterHandler(gameState, log);
	DemoParser demoParser(gameState, log, filterHandler);
	demoParser.parseHeader(demo);

	DemoviewerDialog demoviewerDialog;
	demoviewerDialog.open();

	while (demoviewerDialog.getHandle())
	{
		demoviewerDialog.handleMessages();
	}

	CoUninitialize();
	return 0;
}
