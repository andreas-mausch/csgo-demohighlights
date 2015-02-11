#pragma once

#include <iostream>

class Log
{
private:
	std::ostream &output;
	bool verbose;

	void logVargs(const std::string &format, va_list args);

public:
	Log(std::ostream &output, bool verbose);
	~Log();

	void log(const char *format, ...);
	void logVerbose(const char *format, ...);

};
