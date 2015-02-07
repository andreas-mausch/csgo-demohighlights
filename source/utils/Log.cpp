#include <stdarg.h>

#include "Log.h"
#include "../utils/StringFormat.h"

Log::Log(std::ostream &output, bool verbose)
: output(output), verbose(verbose)
{
}

Log::~Log()
{
}

void Log::log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	logVargs(format, args);
	va_end(args);
}

void Log::logVerbose(const char *format, ...)
{
	if (this->verbose)
	{
		va_list args;
		va_start(args, format);
		logVargs(format, args);
		va_end(args);
	}
}

void Log::logVargs(const std::string &format, va_list args)
{
	output << formatStringVargs(format, args) << std::endl;
}
