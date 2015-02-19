#include "RenderBitmap.h"

void renderBitmap(HDC deviceContext, HBITMAP bitmapHandle, int x, int y)
{
	BITMAP bitmap;
	HDC hdcMem = CreateCompatibleDC(deviceContext);
	HGDIOBJ oldBitmap = SelectObject(hdcMem, bitmapHandle);

	GetObject(bitmapHandle, sizeof(bitmap), &bitmap);
	BLENDFUNCTION blend;
	memset(&blend, 0, sizeof(BLENDFUNCTION));
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	AlphaBlend(deviceContext, x, y, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, blend);

	SelectObject(hdcMem, oldBitmap);
	DeleteDC(hdcMem);
}
