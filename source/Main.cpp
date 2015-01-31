#include <iostream>
#include <fstream>

#include "sdk/demofile.h"
#include "sdk/demofiledump.h"

#include "protobuf/generated/cstrike15_usermessages_public.pb.h"
#include "protobuf/generated/netmessages_public.pb.h"

#include <stdarg.h>  // For va_start, etc.

unsigned int endian_swap(unsigned int x)
{
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

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

  void readBits(void *buffer, int bitCount)
  {
	  int bitsLeft = bitCount;
	  if (bitCount > 8)
	  {
		  readBytes(buffer, bitCount / 8);
		  bitsLeft = bitCount % 8;
	  }

	  for (int targetBit = bitCount - bitsLeft; targetBit < bitCount; targetBit++)
	  {
		  reinterpret_cast<char *>(buffer)[targetBit / 8] |= readBit() << (targetBit % 8);
	  }
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
			  result |= readBit() << i;
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

	unsigned int ReadUBitLong( int numbits )
	{
		unsigned int result = 0;
		for (int i = 0; i < numbits; i++)
		{
			result |= readBit() << i;
		}
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

int s_nNumStringTables = 0;
StringTableData_t s_StringTables[ MAX_STRING_TABLES ];

void parseStringTableUpdate(memstream &stream, int entryCount, int maximumEntries, int userDataSize, int userDataSizeBits, int userDataFixedSize, bool userData)
{
	int nTemp = maximumEntries;
	int nEntryBits = 0;
	while (nTemp >>= 1) ++nEntryBits;
	int lastEntry = -1;

	bool encodeUsingDictionaries = stream.readBit();

	if (encodeUsingDictionaries)
	{
		std::cout << "encodeUsingDictionaries" << std::endl;
		return;
	}

	// std::cout << "parseStringTableUpdate entryCount: " << entryCount << std::endl;
	for (int i = 0; i < entryCount; i++)
	{
		int entryIndex = lastEntry + 1;
		if (!stream.readBit())
		{
			entryIndex = stream.ReadUBitLong(nEntryBits);
		}

		lastEntry = entryIndex;

		if ( entryIndex < 0 || entryIndex >= maximumEntries )
		{
			std::cout << "ParseStringTableUpdate: bogus string index " << entryIndex << std::endl;
			return;
		}

		// std::cout << "entryIndex: " << entryIndex << std::endl;

		const char *pEntry = NULL;
		char entry[ 1024 ]; 
		char substr[ 1024 ];
		entry[ 0 ] = 0;

		if ( stream.readBit() )
		{
			bool substringcheck = stream.readBit();

			if ( substringcheck )
			{
				int index = stream.ReadUBitLong( 5 );
				int bytestocopy = stream.ReadUBitLong( SUBSTRING_BITS );
				std::string substr_str = stream.readString( sizeof( substr ) );
				strncpy(substr, substr_str.c_str(), sizeof(substr));
			}
			else
			{
				std::string entry_str = stream.readString( sizeof( entry ) );
				strncpy(entry, entry_str.c_str(), sizeof(entry));
			}

			pEntry = entry;
		}
		
		// Read in the user data.
		unsigned char tempbuf[ MAX_USERDATA_SIZE ];
		memset( tempbuf, 0, sizeof( tempbuf ) );
		const void *pUserData = NULL;
		int nBytes = 0;

		if ( stream.readBit() )
		{
			if ( userDataFixedSize )
			{
				// Don't need to read length, it's fixed length and the length was networked down already.
				nBytes = userDataSize;
				assert( nBytes > 0 );
				tempbuf[ nBytes - 1 ] = 0; // be safe, clear last byte
				stream.readBits( tempbuf, userDataSizeBits );
			}
			else
			{
				nBytes = stream.ReadUBitLong( MAX_USERDATA_BITS );
				if ( nBytes > sizeof( tempbuf ) )
				{
					printf( "ParseStringTableUpdate: user data too large (%d bytes).", nBytes);
					return;
				}

				stream.readBytes( tempbuf, nBytes );
			}

			pUserData = tempbuf;
		}

		if ( pEntry == NULL )
		{
			pEntry = "";// avoid crash because of NULL strings
		}

		if ( userData && pUserData != NULL )
		{
			const player_info_t *pUnswappedPlayerInfo = ( const player_info_t * )pUserData;
				std::cout << "parseStringTableUpdate player name: " << pUnswappedPlayerInfo->name << " / " << endian_swap(pUnswappedPlayerInfo->userID) << std::endl;
		}
		else
		{
		}
	}
}

void createStringTable(CSVCMsg_CreateStringTable &message)
{
	// std::cout << "svc_CreateStringTable: " << message.name() << std::endl;
	char *data = const_cast<char *>(message.string_data().c_str());
	membuf buffer(data, message.string_data().size());
	memstream stream(buffer);
	parseStringTableUpdate(stream, message.num_entries(), message.max_entries(), message.user_data_size(), message.user_data_size_bits(), message.user_data_fixed_size(), message.name() == "userinfo");

	strncpy(s_StringTables[ s_nNumStringTables ].szName, message.name().c_str(), sizeof( s_StringTables[ s_nNumStringTables ].szName ));
	s_StringTables[ s_nNumStringTables ].nMaxEntries = message.max_entries();
	s_nNumStringTables++;
}

void updateStringTable(CSVCMsg_UpdateStringTable &message)
{
	// std::cout << "svc_UpdateStringTable: " << message.table_id() << std::endl;
	char *data = const_cast<char *>(message.string_data().c_str());
	membuf buffer(data, message.string_data().size());
	memstream stream(buffer);
	parseStringTableUpdate(stream, message.num_changed_entries(), s_StringTables[ message.table_id() ].nMaxEntries, 0, 0, 0, strcmp(s_StringTables[ message.table_id() ].szName, "userinfo") == 0);
}

CSVCMsg_GameEventList _gameEventList;

const CSVCMsg_GameEventList::descriptor_t& findGameEvent(int eventId)
{
	for (int i = 0; i < _gameEventList.descriptors().size(); i++ )
	{
		const CSVCMsg_GameEventList::descriptor_t& Descriptor = _gameEventList.descriptors( i );

		if ( Descriptor.eventid() == eventId )
		{
			return Descriptor;
		}
	}

	throw 1;
}

const CSVCMsg_GameEvent::key_t& getValue(CSVCMsg_GameEvent &message, const CSVCMsg_GameEventList::descriptor_t& descriptor, const std::string &keyName)
{
	for (int i = 0; i < descriptor.keys().size(); i++)
	{
		const CSVCMsg_GameEventList::key_t& key = descriptor.keys(i);
		if (key.name() == keyName)
		{
			return message.keys(i);
		}
	}

	throw 2;
}

std::map<int, std::string> playerNames;

std::string findPlayerName(int userId)
{
	return playerNames.find(userId)->second;
}

void gameEvent(CSVCMsg_GameEvent &message)
{
	const CSVCMsg_GameEventList::descriptor_t& descriptor = findGameEvent(message.eventid());
	// std::cout << "gameEvent: " << descriptor.name() << std::endl;

	if (descriptor.name() == "player_death")
	{
		std::string attacker = findPlayerName(getValue(message, descriptor, "attacker").val_short());
		std::string userid = findPlayerName(getValue(message, descriptor, "userid").val_short());
		std::cout << "gameEvent: " << descriptor.name() << ": " << attacker << " killed " << userid << std::endl;
	}
	else if (descriptor.name() == "round_start")
	{
		std::cout << "gameEvent: " << descriptor.name() << std::endl;
	}
	else if (descriptor.name() == "round_end")
	{
		std::cout << "gameEvent: " << descriptor.name() << " / " << getValue(message, descriptor, "winner").val_byte() << std::endl;
	}
}

void gameEventList(CSVCMsg_GameEventList &message)
{
	std::cout << "gameEventList" << std::endl;
	_gameEventList = message;
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
		case svc_CreateStringTable: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_CreateStringTable message;
			message.ParseFromArray(bytes, messageLength);
			createStringTable(message);
									} break;
		case svc_UpdateStringTable: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_UpdateStringTable message;
			message.ParseFromArray(bytes, messageLength);
			updateStringTable(message);
									} break;
		case svc_GameEvent: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_GameEvent message;
			message.ParseFromArray(bytes, messageLength);
			gameEvent(message);
									} break;
		case svc_GameEventList: {
			char *bytes = new char[messageLength];
			demo.readBytes(bytes, messageLength);
			CSVCMsg_GameEventList message;
			message.ParseFromArray(bytes, messageLength);
			gameEventList(message);
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
	// std::cout << "tableName: " << tableName << std::endl;

	int wordCount = stringtables.readWord();
	// std::cout << "wordCount: " << wordCount << std::endl;

	for (int i = 0; i < wordCount; i++)
	{
		bool userInfo = tableName == "userinfo";
		std::string name = stringtables.readNullTerminatedString(4096);

		bool hasUserData = stringtables.readBit();
		if (hasUserData)
		{
			int userDataLength = stringtables.readWord();
			unsigned char *data = new unsigned char[ userDataLength + 4 ];
			stringtables.readBytes(data, userDataLength);

			if (userInfo)
			{
				const player_info_t *pUnswappedPlayerInfo = ( const player_info_t * )data;
				int userId = endian_swap(pUnswappedPlayerInfo->userID);
				playerNames[userId] = pUnswappedPlayerInfo->name;
				std::cout << "\tplayer name: " << name << " / " << pUnswappedPlayerInfo->name << " / " << userId << std::endl;
			}

			delete[] data;
		}
		else
		{
		}
	}

	bool clientSideData = stringtables.readBit();

	if (clientSideData)
	{
		for (int i = 0; i < wordCount; i++)
		{
			std::string name = stringtables.readNullTerminatedString(4096);
			std::cout << "\tname: " << name << std::endl;

			bool hasUserData = stringtables.readBit();
			if (hasUserData)
			{
				int userDataLength = stringtables.readWord();
				unsigned char *data = new unsigned char[ userDataLength + 4 ];
				stringtables.readBytes(data, userDataLength);
				delete[] data;
			}
			else
			{
			}
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
	std::ifstream demofile("MLGXGA2015-GF-ldlc-vs-nip-dust2.dem", std::ios_base::binary);
	demofile.seekg(0, std::ios::end);
	size_t size = demofile.tellg();
	std::string str(size, ' ');
	demofile.seekg(0);
	demofile.read(&str[0], size); 
	membuf demoBuffer(const_cast<char *>(str.c_str()), str.length());
	memstream demo(demoBuffer);

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
		// std::cout << "command: " << ((int)command) << " at " << position << std::endl;

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
