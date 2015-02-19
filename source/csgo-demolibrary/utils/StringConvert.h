#pragma once

#include <string>

std::wstring toWidestring(const std::string &string);
std::string toString(const std::wstring &wstring);
