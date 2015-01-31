#pragma once

#include <string>

class MemoryStream;

void unhandledCommand(const std::string &description);
void parsePacket(MemoryStream &demo);
void parseDatatables(MemoryStream &demo);
void parseStringtables(MemoryStream &demo);
