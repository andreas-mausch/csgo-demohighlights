#include "Stringtable.h"

Stringtable::Stringtable(const std::string &name, int maxEntries)
: name(name), maxEntries(maxEntries)
{
}

Stringtable::~Stringtable()
{
}

const std::string &Stringtable::getName()
{
	return name;
}

int Stringtable::getMaxEntries()
{
	return maxEntries;
}
