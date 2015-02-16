#pragma once

#include "Dialog.h"

class DemoviewerDialog : public Dialog
{
private:

	void onPaint();

public:
	DemoviewerDialog();
	~DemoviewerDialog();

	virtual INT_PTR callback(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
	bool handleMessages();
};
