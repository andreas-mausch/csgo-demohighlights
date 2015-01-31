#include "DemoParser.h"
#include "GameState.h"

#include "sdk/demofile.h"
#include "sdk/demofiledump.h"

#include "streams/MemoryStream.h"
#include "streams/MemoryBitStream.h"
#include "streams/MemoryStreamBuffer.h"

#include "utils/EndianConverter.h"
#include "utils/StringFormat.h"

void unhandledCommand(const std::string &description)
{
	std::cout << "Unhandled command: " << description << std::endl;
	//exit(1);
}

void serverInfo(const char *bytes, int length)
{
	std::cout << "serverInfo: " << length << std::endl;
	CSVCMsg_ServerInfo serverInfo;
	serverInfo.ParseFromArray(bytes, length);
	delete[] bytes;
	std::cout << serverInfo.DebugString() << std::endl;
}

void parseStringTableUpdate(MemoryBitStream &stream, int entryCount, int maximumEntries, int userDataSize, int userDataSizeBits, int userDataFixedSize, bool userData)
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
				std::string substr_str = stream.readFixedLengthString( sizeof( substr ) );
				strncpy(substr, substr_str.c_str(), sizeof(substr));
			}
			else
			{
				std::string entry_str = stream.readFixedLengthString( sizeof( entry ) );
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

void DemoParser::createStringTable(CSVCMsg_CreateStringTable &message)
{
	// std::cout << "svc_CreateStringTable: " << message.name() << std::endl;
	char *data = const_cast<char *>(message.string_data().c_str());
	MemoryStreamBuffer buffer(data, message.string_data().size());
	MemoryBitStream stream(buffer);
	parseStringTableUpdate(stream, message.num_entries(), message.max_entries(), message.user_data_size(), message.user_data_size_bits(), message.user_data_fixed_size(), message.name() == "userinfo");

	StringTableData_t stringTable;
	strncpy(stringTable.szName, message.name().c_str(), sizeof( stringTable.szName ));
	stringTable.nMaxEntries = message.max_entries();
	gameState.getStringTables().push_back(stringTable);
}

void DemoParser::updateStringTable(CSVCMsg_UpdateStringTable &message)
{
	// std::cout << "svc_UpdateStringTable: " << message.table_id() << std::endl;
	char *data = const_cast<char *>(message.string_data().c_str());
	MemoryStreamBuffer buffer(data, message.string_data().size());
	MemoryBitStream stream(buffer);

	StringTableData_t &stringTable = gameState.getStringTables()[message.table_id()];
	parseStringTableUpdate(stream, message.num_changed_entries(), stringTable.nMaxEntries, 0, 0, 0, strcmp(stringTable.szName, "userinfo") == 0);
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

void DemoParser::gameEvent(CSVCMsg_GameEvent &message)
{
	const CSVCMsg_GameEventList::descriptor_t& descriptor = gameState.getGameEvent(message.eventid());
	// std::cout << "gameEvent: " << descriptor.name() << std::endl;

	if (descriptor.name() == "player_death")
	{
		std::string attacker = gameState.findPlayerByUserId(getValue(message, descriptor, "attacker").val_short()).getName();
		std::string userid = gameState.findPlayerByUserId(getValue(message, descriptor, "userid").val_short()).getName();
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

void DemoParser::gameEventList(CSVCMsg_GameEventList &message)
{
	std::cout << "gameEventList" << std::endl;
	gameState.setGameEvents(message);
}

void DemoParser::parsePacket2(MemoryStream &demo, int length)
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

void DemoParser::parseStringtable(MemoryBitStream &stringtables)
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
				Player player(userId, pUnswappedPlayerInfo->name);
				gameState.getPlayers().push_back(player);
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

DemoParser::DemoParser(GameState &gameState)
: gameState(gameState)
{
}

DemoParser::~DemoParser()
{
}

DemoHeader DemoParser::parseHeader(MemoryStream &demo)
{
	DemoHeader header;
	header.filestamp = demo.readFixedLengthString(8);
	header.protocol = demo.readInt();
	header.networkProtocol = demo.readInt();
	header.serverName = demo.readFixedLengthString(MAX_OSPATH);
	header.clientName = demo.readFixedLengthString(MAX_OSPATH);
	header.mapName = demo.readFixedLengthString(MAX_OSPATH);
	header.gameDirectory = demo.readFixedLengthString(MAX_OSPATH);
	header.playbackTime = demo.readFloat();
	header.playbackTicks = demo.readInt();
	header.playbackFrames = demo.readInt();
	header.signonlength = demo.readInt();

	return header;
}

void DemoParser::parsePacket(MemoryStream &demo)
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

void DemoParser::parseDatatables(MemoryStream &demo)
{
	int length = demo.readInt();
	std::cout << "Parse datatables: " << length << std::endl;
	char *datatablesBytes = new char[length];
	demo.readBytes(datatablesBytes, length);
	delete[] datatablesBytes;
}

void DemoParser::parseStringtables(MemoryStream &demo)
{
	int length = demo.readInt();
	std::cout << "Parse stringtables: " << length << std::endl;
	char *stringtablesBytes = new char[length];
	demo.readBytes(stringtablesBytes, length);
	MemoryStreamBuffer stringtablesBuffer(stringtablesBytes, length);
	MemoryBitStream stringtables(stringtablesBuffer);
	int tableCount = stringtables.readByte();
	std::cout << "Parse stringtables tableCount: " << tableCount << std::endl;

	for (int i = 0; i < tableCount; i++)
	{
		parseStringtable(stringtables);
	}

	delete[] stringtablesBytes;
}

bool DemoParser::parseNextTick(MemoryStream &demo)
{
	unsigned char command = demo.readByte();
	gameState.setTick(demo.readInt());
	unsigned char playerSlot = demo.readByte();

	switch (command)
	{
	case dem_signon:
	case dem_packet:
		parsePacket(demo);
		break;
	case dem_synctick:
		break;
	case dem_consolecmd:
		unhandledCommand(formatString("command: default %d", command));
		break;
	case dem_usercmd:
		unhandledCommand(formatString("command: default %d", command));
		break;
	case dem_datatables:
		parseDatatables(demo);
		break;
	case dem_stop:
		return false;
	case dem_customdata:
		unhandledCommand(formatString("command: default %d", command));
		break;
	case dem_stringtables:
		parseStringtables(demo);
		break;
	default:
		unhandledCommand(formatString("command: default %d", command));
	}

	gameState.setPositionInStream(demo.tellg());

	// std::cout << "GameState: " << tick << " / " << positionInStream << std::endl;

	return true;
}
