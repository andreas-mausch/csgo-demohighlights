#include <iostream>
#include <fstream>

#include "File.h"

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str(), std::ios_base::binary);

	if (!file)
	{
		return "";
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string buffer(size, ' ');
	file.seekg(0);
	file.read(&buffer[0], size);
	return buffer;
}
