#pragma once

#include <windows.h>

void renderBitmap(HDC deviceContext, HBITMAP bitmapHandle, int x, int y);
void renderBitmapStretched(HDC deviceContext, HBITMAP bitmapHandle, int x, int y, int width, int height);
