#pragma once

#include <string>
#include <vector>

class Stringtable
{
private:
	std::vector<std::string> strings;
public:
	std::string getString(int index);
	void addString(const std::string &string);
	void updateString(int index, const std::string &string);
};
