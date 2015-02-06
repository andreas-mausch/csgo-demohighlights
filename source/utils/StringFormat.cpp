#include <stdarg.h>
#include "StringFormat.h"

std::string formatString(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	std::string result = formatStringVargs(format, args);
	va_end(args);
	return result;
}

std::string formatStringVargs(const std::string &format, va_list args)
{
	int size = ((int)format.size()) * 2 + 50;   // Use a rubric appropriate for your code
	std::string str;
	while (1) {     // Maximum two passes on a POSIX system...
		str.resize(size);
		int n = vsnprintf_s((char *)str.data(), size, size, format.c_str(), args);
		if (n > -1 && n < size) {  // Everything worked
			str.resize(n);
			return str;
		}
		if (n > -1)  // Needed size returned
			size = n + 1;   // For null char
		else
			size *= 2;      // Guess at a larger size (OS specific)
	}
	return str;
}
