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

		// std::cout << "\tentry index: " << entryIndex << "; lastEntry: " << lastEntry << "; maximum entries: " << maximumEntries << std::endl;

		lastEntry = entryIndex;

		if ( entryIndex < 0 || entryIndex >= maximumEntries )
		{
			std::cout << "ParseStringTableUpdate: bogus string index " << entryIndex << std::endl;
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
				// std::cout << "\tsubstr: " << substr_str << "; old: " << existingEntry << "; new: " << entry << std::endl;
			}
			else
			{
				std::string entry_str = stream.readNullTerminatedString(sizeof(entry));
				// std::cout << "\tentry_str: " << entry_str << std::endl;
				strncpy_s(entry, entry_str.c_str(), sizeof(entry));
			}

			pEntry = entry;
		}
		// std::cout << "\tmemory bit stream 3: " << stream.tellg() << "/" << stream.getCurrentBitPosition() << " - " << stream.eof() << stream.fail() << stream.bad() << std::endl;

		// Read in the user data.
		unsigned char tempbuf[ MAX_USERDATA_SIZE ];
		memset( tempbuf, 0, sizeof( tempbuf ) );
		const void *pUserData = NULL;
		int nBytes = 0;

		if ( stream.readBit() )
		{
			// std::cout << "\tmemory bit stream 4: " << stream.tellg() << "/" << stream.getCurrentBitPosition() << " - " << stream.eof() << stream.fail() << stream.bad() << " - " << (int)stream.getCurrentByte() << std::endl;
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

				// std::cout << "\tmemory bit stream 5: " << stream.tellg() << "/" << stream.getCurrentBitPosition() << " - " << stream.eof() << stream.fail() << stream.bad() << " - " << nBytes << "/" << MAX_USERDATA_SIZE << std::endl;
				stream.readBytes( tempbuf, nBytes );
				// std::cout << "\tmemory bit stream 6: " << stream.tellg() << "/" << stream.getCurrentBitPosition() << " - " << stream.eof() << stream.fail() << stream.bad() << std::endl;
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
			// std::cout << "parseStringTableUpdate player name: " << pUnswappedPlayerInfo->name << " / " << endian_swap(pUnswappedPlayerInfo->userID) << std::endl;
			updatePlayer(entryIndex, pUnswappedPlayerInfo);
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
	// std::cout << "svc_CreateStringTable: " << message.name() << " (" << message.string_data().size() << " bytes)" << std::endl;
	MemoryBitStream stream(message.string_data().c_str(), message.string_data().size());
	parseStringTableUpdate(stream, message.num_entries(), message.max_entries(), message.user_data_size(), message.user_data_size_bits(), message.user_data_fixed_size(), message.name() == "userinfo");

	Stringtable stringTable(message.name(), message.max_entries());
	gameState.getStringTables().push_back(stringTable);

	// if (strcmp(stringTable.szName, "userinfo") == 0)
	hexdump(message.string_data().c_str(), message.string_data().size());
}

void DemoParser::updateStringTable(CSVCMsg_UpdateStringTable &message)
{
	// std::cout << "svc_UpdateStringTable: " << message.table_id() << " (" << message.string_data().size() << " bytes)" << std::endl;
	char *data = const_cast<char *>(message.string_data().c_str());
	MemoryBitStream stream(message.string_data().c_str(), message.string_data().size());

	Stringtable &stringTable = gameState.getStringTables()[message.table_id()];

	// if (strcmp(stringTable.szName, "userinfo") == 0)
	// hexdump(message.string_data().c_str(), message.string_data().size());

	try
	{
		parseStringTableUpdate(stream, message.num_changed_entries(), stringTable.getMaxEntries(), 0, 0, 0, stringTable.getName() == "userinfo");
	}
	catch (...)
	{
		if (stringTable.getName() != "soundprecache")
		{
			std::cout << "ERROR: updateStringTable() failed: " << stringTable.getName() << std::endl;
			hexdump(message.string_data().c_str(), message.string_data().size());
		}
	}
}
