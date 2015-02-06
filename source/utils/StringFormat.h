#pragma once

#include <string>

std::string formatString(const char *format, ...);
std::string formatStringVargs(const std::string &format, va_list args);
