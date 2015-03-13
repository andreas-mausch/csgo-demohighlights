#include "DemoParser.h"
#include "../gamestate/GameState.h"
#include "../protobuf/generated/netmessages_public.pb.h"
#include "../streams/MemoryBitStream.h"
#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"

bool DemoParser::ParseDataTable( MemoryBitStream &buf )
{
	CSVCMsg_SendTable msg;
	while ( 1 )
	{
		int type = buf.readVarInt32();

		int size = buf.readVarInt32();
		void *pBuffer = new char[size];
		buf.readBytes(pBuffer, size);
		msg.ParseFromArray( pBuffer, size );
		delete[] pBuffer;

		if ( msg.is_end() )
			break;

		gameState.s_DataTables.push_back( msg );
	}

	short nServerClasses = buf.readWord();
	assert( nServerClasses );
	for ( int i = 0; i < nServerClasses; i++ )
	{
		ServerClass entry;
		entry.classId = buf.readWord();
		if ( entry.classId >= nServerClasses )
		{
			printf( "ParseDataTable: invalid class index (%d).\n", entry.classId);
			return false;
		}

		entry.name = buf.readNullTerminatedString(256);
		entry.datatableName = buf.readNullTerminatedString(256);

		// find the data table by name
		entry.dataTableId = -1;
		for ( unsigned int j = 0; j < gameState.s_DataTables.size(); j++ )
		{
			if (entry.datatableName == gameState.s_DataTables[ j ].net_table_name())
			{
				entry.dataTableId = j;
				break;
			}
		}

		gameState.serverClasses.push_back( entry );
	}


	for ( int i = 0; i < nServerClasses; i++ )
	{
		FlattenDataTable( i );
	}

	// perform integer log2() to set s_nServerClassBits
	int nTemp = nServerClasses;
	gameState.s_nServerClassBits = 0;
	while (nTemp >>= 1) ++gameState.s_nServerClassBits;

	gameState.s_nServerClassBits++;

	return true;
}

void DemoParser::FlattenDataTable( int nServerClass )
{
	CSVCMsg_SendTable *pTable = &gameState.s_DataTables[ gameState.serverClasses[ nServerClass ].dataTableId ];

	gameState.s_currentExcludes.clear();
	GatherExcludes( pTable );

	GatherProps( pTable, nServerClass );

	std::vector<PropertyKey> &flattenedProps = gameState.serverClasses[ nServerClass ].keys;

	// get priorities
	std::vector< uint32 > priorities;
	priorities.push_back(64);
	for ( unsigned int i = 0; i < flattenedProps.size(); i++ )
	{
		uint32 priority = flattenedProps[ i ].priority;

		bool bFound = false;
		for ( uint32 j = 0; j < priorities.size(); j++ )
		{
			if ( priorities[ j ] == priority )
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			priorities.push_back(priority);
		}
	}

	std::sort(priorities.begin(), priorities.end());

	// sort flattenedProps by priority
	uint32 start = 0;
	for (uint32 priority_index = 0; priority_index < priorities.size(); ++priority_index)
	{
		uint32 priority = priorities[priority_index];

		while( true )
		{
			uint32 currentProp = start;
			while (currentProp < flattenedProps.size()) 
			{
				const PropertyKey &key = flattenedProps[currentProp];

				if (key.priority == priority || (priority == 64 && (SPROP_CHANGES_OFTEN & key.flags))) 
				{
					if ( start != currentProp )
					{
						PropertyKey temp = flattenedProps[start];
						flattenedProps[start] = flattenedProps[currentProp];
						flattenedProps[currentProp] = temp;
					}
					start++;
					break;
				}
				currentProp++;
			}

			if ( currentProp == flattenedProps.size() )
				break;
		}
	}
}

void DemoParser::GatherExcludes( CSVCMsg_SendTable *pTable )
{
	for ( int iProp=0; iProp < pTable->props_size(); iProp++ )
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props( iProp );
		if ( sendProp.flags() & SPROP_EXCLUDE )
		{
			gameState.s_currentExcludes.push_back( ExcludeEntry( sendProp.var_name().c_str(), sendProp.dt_name().c_str(), pTable->net_table_name().c_str() ) );
		}

		if ( sendProp.type() == DPT_DataTable )
		{
			CSVCMsg_SendTable *pSubTable = GetTableByName( sendProp.dt_name().c_str() );
			if ( pSubTable != NULL )
			{
				GatherExcludes( pSubTable );
			}
		}
	}
}

void DemoParser::GatherProps( CSVCMsg_SendTable *pTable, int nServerClass )
{
	std::vector< PropertyKey > tempFlattenedProps;
	GatherProps_IterateProps( pTable, nServerClass, tempFlattenedProps );

	std::vector<PropertyKey> &flattenedProps = gameState.serverClasses[ nServerClass ].keys;
	for ( uint32 i = 0; i < tempFlattenedProps.size(); i++ )
	{
		flattenedProps.push_back( tempFlattenedProps[ i ] );
	}
}

PropertyKey propertyKeyFromSendProp(CSVCMsg_SendTable *pTable, int iProp)
{
	const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props( iProp );

	if ( sendProp.type() == DPT_Array )
	{
		PropertyKey arrayType = propertyKeyFromSendProp(pTable, iProp - 1);
		return PropertyKey(sendProp.var_name(), sendProp.priority(), sendProp.flags(), sendProp.type(), sendProp.num_elements(), sendProp.num_bits(), sendProp.high_value(), sendProp.low_value(), &arrayType);
	}
	else
	{
		return PropertyKey(sendProp.var_name(), sendProp.priority(), sendProp.flags(), sendProp.type(), 1, sendProp.num_bits(), sendProp.high_value(), sendProp.low_value(), NULL);
	}
}

void DemoParser::GatherProps_IterateProps( CSVCMsg_SendTable *pTable, int nServerClass, std::vector<PropertyKey> &flattenedProps )
{
	for ( int iProp=0; iProp < pTable->props_size(); iProp++ )
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props( iProp );

		if ( ( sendProp.flags() & SPROP_INSIDEARRAY ) || 
			( sendProp.flags() & SPROP_EXCLUDE ) || 
			IsPropExcluded( pTable, sendProp ) )
		{
			continue;
		}

		if ( sendProp.type() == DPT_DataTable )
		{
			CSVCMsg_SendTable *pSubTable = GetTableByName( sendProp.dt_name().c_str() );
			if ( pSubTable != NULL )
			{
				if ( sendProp.flags() & SPROP_COLLAPSIBLE )
				{
					GatherProps_IterateProps( pSubTable, nServerClass, flattenedProps );
				}
				else
				{
					GatherProps( pSubTable, nServerClass );
				}
			}
		}
		else
		{
			flattenedProps.push_back(propertyKeyFromSendProp(pTable, iProp));
		}
	}
}

bool DemoParser::IsPropExcluded( CSVCMsg_SendTable *pTable, const CSVCMsg_SendTable::sendprop_t &checkSendProp )
{
	for ( unsigned int i = 0; i < gameState.s_currentExcludes.size(); i++ )
	{
		if ( pTable->net_table_name().compare( gameState.s_currentExcludes[ i ].m_pDTName ) == 0 &&
			checkSendProp.var_name().compare( gameState.s_currentExcludes[ i ].m_pVarName ) == 0 )
		{
			return true;
		}
	}
	return false;
}

CSVCMsg_SendTable *DemoParser::GetTableByName( const char *pName )
{
	for ( unsigned int i = 0; i < gameState.s_DataTables.size(); i++ )
	{
		if ( gameState.s_DataTables[ i ].net_table_name().compare( pName ) == 0 )
		{
			return &(gameState.s_DataTables[ i ]);
		}
	}
	return NULL;
}

CSVCMsg_SendTable *DemoParser::GetTableByClassID( uint32 nClassID )
{
	for ( uint32 i = 0; i < gameState.serverClasses.size(); i++ )
	{
		if ( gameState.serverClasses[ i ].classId == nClassID )
		{
			return &(gameState.s_DataTables[ gameState.serverClasses[i].dataTableId ]);
		}
	}
	return NULL;
}

PropertyKey *DemoParser::GetSendPropByIndex( uint32 uClass, uint32 uIndex )
{
	if ( uIndex < gameState.serverClasses[ uClass ].keys.size() )
	{
		return &gameState.serverClasses[ uClass ].keys[ uIndex ];
	}
	return NULL;
}
