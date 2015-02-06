#include "Stringtable.h"

std::string Stringtable::getString(int index)
{
	return strings.at(index);
}

void Stringtable::addString(const std::string &string)
{
	strings.push_back(string);
}

void Stringtable::updateString(int index, const std::string &string)
{
	strings[index] = string;
}
