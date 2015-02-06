#pragma once

#include <string>
#include <vector>

class Stringtable
{
private:
	std::string name;
	int maxEntries;

public:
	Stringtable(const std::string &name, int maxEntries);
	~Stringtable();

	const std::string &getName();
	int getMaxEntries();
};
