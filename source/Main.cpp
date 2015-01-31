#include <iostream>
#include <fstream>

#include "sdk/demofile.h"

#include "protobuf/generated/cstrike15_usermessages_public.pb.h"
#include "protobuf/generated/netmessages_public.pb.h"

#include <stdarg.h>  // For va_start, etc.

class membuf : public std::basic_streambuf<char>
{
private:
	char *buffer;
public:
  membuf(char* p, size_t n) {
	setg(p, p, p + n);
	// setp(p, p + n);
  }

  pos_type seekoff( off_type off, std::ios_base::seekdir dir,
                          std::ios_base::openmode which = std::ios_base::in | std::ios_base::out )
  {
	  int newPosition = off;

	  if (dir == std::ios_base::cur)
	  {
		  newPosition += gptr() - eback();
	  }

	  if (dir == std::ios_base::end)
	  {
		  newPosition += egptr() - eback();
	  }

	  setg(eback(), eback() + newPosition, egptr());
	  return newPosition;
  }

  pos_type seekpos( pos_type pos,
	  std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
  {
	  setg(eback(), eback() + pos, egptr());
	  return pos;
  }
};

class memstream : public std::istream
{
private:
	membuf &buffer;
	int currentBitPosition;
	char currentByte;
public:
	memstream(membuf &buffer) : buffer(buffer), std::istream(&buffer) {
	  currentBitPosition = 0;
	  currentByte = 0;
	}

  bool readBit()
  {
	  if (currentBitPosition == 0)
	  {
		  read(&currentByte, sizeof(char));
	  }

	  bool bit = (currentByte >> currentBitPosition) & 0x01;
	  currentBitPosition++;
	  if (currentBitPosition == 8)
	  {
		  currentBitPosition = 0;
	  }
	  return bit;
  }

  char readByte()
  {
	  char result = 0;

	  if (currentBitPosition == 0)
	  {
		  read(&result, sizeof(char));
	  }
	  else
	  {
		  for (int i = 0; i < 8; i++)
		  {
			  result |= (readBit() << i);
		  }
	  }

	  return result;
  }

  void readBytes(void *buffer, int length)
  {
	  if (currentBitPosition == 0)
	  {
		  read(reinterpret_cast<char *>(buffer), length);
	  }
	  else
	  {
		  for (int i = 0; i < length; i++)
		  {
			  reinterpret_cast<char *>(buffer)[i] = readByte();
		  }
	  }
  }

	std::string readString(int length)
	{
		char *buffer = new char[length];
		readBytes(buffer, length);
		std::string result(buffer);
		delete[] buffer;

		return result;
	}

	std::string readNullTerminatedString(int maximumLength)
	{
		char *buffer = new char[maximumLength];
		memset(buffer, 0, maximumLength);

		bool bTooSmall = false;
		int iChar = 0;
		while(1)
		{
			char val = readByte();
			if ( val == 0 )
				break;

			if ( iChar < ( maximumLength - 1 ) )
			{
				buffer[ iChar ] = val;
				++iChar;
			}
			else
			{
				bTooSmall = true;
			}
		}

		std::string result(buffer);
		delete[] buffer;
		return result;
	}

	short readWord()
	{
		short result;
		readBytes(&result, sizeof(short));
		return result;
	}

	int readInt()
	{
		int result;
		readBytes(&result, sizeof(int));
		return result;
	}

	float readFloat()
	{
		float result;
		readBytes(&result, sizeof(float));
		return result;
	}

	int readVarInt32()
	{
		int maximumBytes = sizeof(int);
		int result = 0;
		unsigned char b = 0;
		int currentByte = 0;

		do
		{
			if (currentByte + 1 == maximumBytes)
			{
				return result;
			}

			b = readByte();

			result |= (b & 0x7F) << (7 * currentByte);
			currentByte++;
		} while (b & 0x80);

		return result;
	}
};

std::string string_format(const std::string fmt, ...) {
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
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

void unhandledCommand(const std::string &description)
{
	std::cout << "Unhandled command: " << description << std::endl;
	//exit(1);
}

struct DemoHeader
{
	std::string filestamp;
	int protocol;
	int networkProtocol;
	std::string serverName;
	std::string clientName;
	std::string mapName;
	std::string gameDirectory;
	float playbackTime;
	int playbackTicks;
	int playbackFrames;
	int signonlength;
};

void parseHeader(memstream &demo)
{
	DemoHeader header;
	header.filestamp = demo.readString(8);
	header.protocol = demo.readInt();
	header.networkProtocol = demo.readInt();
	header.serverName = demo.readString(MAX_OSPATH);
	header.clientName = demo.readString(MAX_OSPATH);
	header.mapName = demo.readString(MAX_OSPATH);
	header.gameDirectory = demo.readString(MAX_OSPATH);
	header.playbackTime = demo.readFloat();
	header.playbackTicks = demo.readInt();
	header.playbackFrames = demo.readInt();
	header.signonlength = demo.readInt();
}

void serverInfo(const char *bytes, int length)
{
	std::cout << "serverInfo: " << length << std::endl;
	CSVCMsg_ServerInfo serverInfo;
	serverInfo.ParseFromArray(bytes, length);
	delete[] bytes;
	std::cout << serverInfo.DebugString() << std::endl;
}

void parsePacket2(memstream &demo, int length)
{
	int destination = (int)demo.tellg() + length;

	while (demo.tellg() < destination)
	{
		int command = demo.readVarInt32();
		int messageLength = demo.readVarInt32();

		switch (command)
		{
		case net_Tick:
			demo.seekg(messageLength, std::ios_base::cur);
			break;
		case svc_ServerInfo: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			serverInfo(bytes, messageLength);
			} break;
		default:
			// unhandledCommand(string_format("message command: default %d", command));
			demo.seekg(messageLength, std::ios_base::cur);
		}
	}

	demo.seekg(destination, std::ios_base::beg);
}

void parsePacket(memstream &demo)
{
	int position = demo.tellg();
	democmdinfo_t cmdinfo;
	demo.readBytes(&cmdinfo, sizeof(democmdinfo_t));
	int sequenceNumberIn = demo.readInt();
	int sequenceNumberOut = demo.readInt();
	int length = demo.readInt();
	// std::cout << "Parse packet. Length: " << length << " at " << position << " / " << demo.tellg() << std::endl;
	parsePacket2(demo, length);
}

void parseDatatables(memstream &demo)
{
	int length = demo.readInt();
	std::cout << "Parse datatables: " << length << std::endl;
	char *datatablesBytes = new char[length];
	demo.readBytes(datatablesBytes, length);
	delete[] datatablesBytes;
}

void parseStringtable(memstream &stringtables)
{
	std::string tableName = stringtables.readNullTerminatedString(256);
	std::cout << "tableName: " << tableName << std::endl;

	if (tableName == "userinfo")
	{
		int wordCount = stringtables.readWord();
		std::cout << "wordCount: " << wordCount << std::endl;

		for (int i = 0; i < wordCount; i++)
		{
			std::string name = stringtables.readNullTerminatedString(4096);
		}
	}
}

void parseStringtables(memstream &demo)
{
	int length = demo.readInt();
	std::cout << "Parse stringtables: " << length << std::endl;
	char *stringtablesBytes = new char[length];
	demo.readBytes(stringtablesBytes, length);
	membuf stringtablesBuffer(stringtablesBytes, length);
	memstream stringtables(stringtablesBuffer);
	int tableCount = stringtables.readByte();
	std::cout << "Parse stringtables tableCount: " << tableCount << std::endl;

	for (int i = 0; i < tableCount; i++)
	{
		parseStringtable(stringtables);
	}

	delete[] stringtablesBytes;
}

int main()
{
	std::ifstream demofile("demo.dem", std::ios_base::binary);
	demofile.seekg(0, std::ios::end);
	size_t size = demofile.tellg();
	std::string str(size, ' ');
	demofile.seekg(0);
	demofile.read(&str[0], size); 
	membuf demoBuffer(const_cast<char *>(str.c_str()), str.length());
	memstream demo(demoBuffer);

	std::cout << "BLA "<< demo.tellg() << std::endl;

	parseHeader(demo);

	int messageCount = 0;
	bool end = false;
	while (!end)
	{
		int position = demo.tellg();
		unsigned char command = demo.readByte();
		int tick = demo.readInt();
		unsigned char playerSlot = demo.readByte();
		messageCount++;
		std::cout << "command: " << ((int)command) << " at " << position << std::endl;

		switch (command)
		{
		case dem_signon:
		case dem_packet:
			parsePacket(demo);
			break;
		case dem_synctick:
			break;
		case dem_consolecmd:
			unhandledCommand(string_format("command: default %d", command));
			break;
		case dem_usercmd:
			unhandledCommand(string_format("command: default %d", command));
			break;
		case dem_datatables:
			parseDatatables(demo);
			break;
		case dem_stop:
			end = true;
			break;
		case dem_customdata:
			unhandledCommand(string_format("command: default %d", command));
			break;
		case dem_stringtables:
			parseStringtables(demo);
			break;
		default:
			unhandledCommand(string_format("command: default %d", command));
		}
	}

	std::cout << "message count: " << messageCount << std::endl;

	return 0;
}
