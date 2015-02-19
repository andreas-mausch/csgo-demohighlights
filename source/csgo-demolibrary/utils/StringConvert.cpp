#include "StringConvert.h"

#include <windows.h>

std::wstring toWidestring(const std::string &string)
{
	std::wstring wstring;
	WCHAR *szWText = new WCHAR[string.length() + 1];
	szWText[string.size()] = '\0';
	MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, szWText, static_cast<int>(string.length()));
	wstring = szWText;
	delete[] szWText;
	return wstring;
}

std::string toString(const std::wstring &wstring)
{
	std::string string;
	int size = WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, NULL, 0, NULL, NULL);
	char *szText = new char[size];
	memset(szText, 0, size);
	WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, szText, size, NULL, NULL);
	string = szText;
	delete[] szText;
	return string;
}
