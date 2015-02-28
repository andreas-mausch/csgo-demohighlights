#include "DemoParser.h"
#include "Stringtable.h"

#include "../gamestate/GameState.h"
#include "../streams/MemoryBitStream.h"
#include "../sdk/demofiledump.h"
#include "../utils/HexDump.h"

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

void DemoParser::parseStringTableUpdate(MemoryBitStream &stream, int entryCount, int maximumEntries, int userDataSize, int userDataSizeBits, int userDataFixedSize, bool userData)
{
	int nTemp = maximumEntries;
	int nEntryBits = 0;
	int lastEntry = -1;

	while (nTemp >>= 1)
	{
		++nEntryBits;
	}

	bool encodeUsingDictionaries = stream.readBit();

	if (encodeUsingDictionaries)
	{
		log.log("encodeUsingDictionaries");
		return;
	}

	log.logVerbose("parseStringTableUpdate entryCount: %d", entryCount);
	for (int i = 0; i < entryCount; i++)
	{
		int entryIndex = lastEntry + 1;
		if (!stream.readBit())
		{
			entryIndex = stream.ReadUBitLong(nEntryBits);
		}

		log.logVerbose("\tentry index: %d; lastEntry: %d; maximum entries: %d", entryIndex,lastEntry, maximumEntries);

		lastEntry = entryIndex;

		if ( entryIndex < 0 || entryIndex >= maximumEntries )
		{
			log.log("ParseStringTableUpdate: bogus string index %d", entryIndex);
			return;
		}

		const char *pEntry = NULL;
		char entry[ 1024 ];
		char substr[ 1024 ];
		entry[0] = 0;

		struct StringHistoryEntry
		{
			char string[1024];
		};
		std::vector< StringHistoryEntry > history;

		if (stream.readBit())
		{
			bool substringcheck = stream.readBit();

			if (substringcheck)
			{
				int index = stream.ReadUBitLong(5);
				int bytestocopy = stream.ReadUBitLong(SUBSTRING_BITS);
				std::string existingEntry;
				if (index < history.size())
				{
					std::string existingEntry = history[index].string;
				}
				strncpy_s(entry, existingEntry.c_str(), bytestocopy + 1);
				std::string substr_str = stream.readNullTerminatedString(sizeof(substr));
				strncat_s(entry, substr_str.c_str(), sizeof(entry));
				// log(true, "\tsubstr: %s; old: %s; new: %s", substr_str.c_str(), existingEntry.c_str(), entry);
			}
			else
			{
				std::string entry_str = stream.readNullTerminatedString(sizeof(entry));
				log.logVerbose("\tentry_str: %s", entry_str.c_str());
				strncpy_s(entry, entry_str.c_str(), sizeof(entry));
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
			log.logVerbose("parseStringTableUpdate player name: %s / %d", pUnswappedPlayerInfo->name, pUnswappedPlayerInfo->userID);
			updatePlayer(entryIndex + 1, pUnswappedPlayerInfo);
		}
		else
		{
		}

		if ( history.size() > 31 )
		{
			history.erase( history.begin() );
		}

		StringHistoryEntry she;
		strncpy_s(she.string, pEntry, sizeof(she.string));
		history.push_back( she );
	}
}

void DemoParser::createStringTable(CSVCMsg_CreateStringTable &message)
{
	log.logVerbose("svc_CreateStringTable: %s (%d bytes)", message.name().c_str(), message.string_data().size());
	MemoryBitStream stream(message.string_data().c_str(), message.string_data().size());
	parseStringTableUpdate(stream, message.num_entries(), message.max_entries(), message.user_data_size(), message.user_data_size_bits(), message.user_data_fixed_size(), message.name() == "userinfo");

	Stringtable stringTable(message.name(), message.max_entries());
	gameState.getStringTables().push_back(stringTable);
}

void DemoParser::updateStringTable(CSVCMsg_UpdateStringTable &message)
{
	log.logVerbose("svc_UpdateStringTable: %d (%d bytes)", message.table_id(), message.string_data().size());
	char *data = const_cast<char *>(message.string_data().c_str());
	MemoryBitStream stream(message.string_data().c_str(), message.string_data().size());

	Stringtable &stringTable = gameState.getStringTables()[message.table_id()];

	try
	{
		parseStringTableUpdate(stream, message.num_changed_entries(), stringTable.getMaxEntries(), 0, 0, 0, stringTable.getName() == "userinfo");
	}
	catch (...)
	{
		if (stringTable.getName() != "soundprecache")
		{
			log.log("ERROR: updateStringTable() failed: %s", stringTable.getName().c_str());
			hexdump(message.string_data().c_str(), message.string_data().size());
		}
	}
}

void DemoParser::parseStringtables(MemoryStream &demo)
{
	int length = demo.readInt();
	log.logVerbose("parseStringtables: %d", length);
	char *stringtablesBytes = new char[length];
	demo.readBytes(stringtablesBytes, length);
	MemoryBitStream stringtables(stringtablesBytes, length);
	int tableCount = stringtables.readByte();
	log.logVerbose("Parse stringtables tableCount: %d", tableCount);

	for (int i = 0; i < tableCount; i++)
	{
		parseStringtable(stringtables);
	}

	delete[] stringtablesBytes;
}

void DemoParser::parseStringtable(MemoryBitStream &stringtables)
{
	std::string tableName = stringtables.readNullTerminatedString(256);
	int wordCount = stringtables.readWord();
	bool userInfo = tableName == "userinfo";

	log.logVerbose("parseStringtable: %s / %d", tableName.c_str(), wordCount);

	for (int i = 0; i < wordCount; i++)
	{
		std::string name = stringtables.readNullTerminatedString(4096);

		bool hasUserData = stringtables.readBit();
		if (hasUserData)
		{
			int userDataLength = stringtables.readWord();
			unsigned char *data = new unsigned char[ userDataLength + 4 ];
			stringtables.readBytes(data, userDataLength);

			if (userInfo)
			{
				updatePlayer(i + 1, reinterpret_cast<const player_info_t *>(data));
			}

			delete[] data;
		}
	}

	bool clientSideData = stringtables.readBit();

	if (clientSideData)
	{
		for (int i = 0; i < wordCount; i++)
		{
			std::string name = stringtables.readNullTerminatedString(4096);
			log.logVerbose("\tname: ", name.c_str());

			bool hasUserData = stringtables.readBit();
			if (hasUserData)
			{
				int userDataLength = stringtables.readWord();
				unsigned char *data = new unsigned char[userDataLength + 4];
				stringtables.readBytes(data, userDataLength);
				delete[] data;
			}
		}
	}
}
