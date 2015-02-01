#include "Datatable.h"

#include "../streams/MemoryBitStream.h"

#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"
#include "../protobuf/generated/netmessages_public.pb.h"

std::vector< CSVCMsg_SendTable > s_DataTables;
std::vector< ServerClass_t > s_ServerClasses;
int s_nServerClassBits = 0;
std::vector< ExcludeEntry > s_currentExcludes;

bool ParseDataTable( MemoryBitStream &buf );
void FlattenDataTable( int nServerClass );
void GatherExcludes( CSVCMsg_SendTable *pTable );
void GatherProps( CSVCMsg_SendTable *pTable, int nServerClass );
void GatherProps_IterateProps( CSVCMsg_SendTable *pTable, int nServerClass, std::vector< FlattenedPropEntry > &flattenedProps );
bool IsPropExcluded( CSVCMsg_SendTable *pTable, const CSVCMsg_SendTable::sendprop_t &checkSendProp );
CSVCMsg_SendTable *GetTableByName( const char *pName );

bool ParseDataTable( MemoryBitStream &buf )
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

		s_DataTables.push_back( msg );
	}

	short nServerClasses = buf.readWord();
	assert( nServerClasses );
	for ( int i = 0; i < nServerClasses; i++ )
	{
		ServerClass_t entry;
		entry.nClassID = buf.readWord();
		if ( entry.nClassID >= nServerClasses )
		{
			printf( "ParseDataTable: invalid class index (%d).\n", entry.nClassID);
			return false;
		}

		std::string name = buf.readNullTerminatedString(sizeof( entry.strName ));
		std::string dtName = buf.readNullTerminatedString(sizeof( entry.strDTName ));
		strncpy_s(entry.strName, name.c_str(), sizeof( entry.strName ));
		strncpy_s(entry.strDTName, dtName.c_str(), sizeof( entry.strDTName ));

		// find the data table by name
		entry.nDataTable = -1;
		for ( unsigned int j = 0; j < s_DataTables.size(); j++ )
		{
			if ( strcmp( entry.strDTName, s_DataTables[ j ].net_table_name().c_str() ) == 0 )
			{
				entry.nDataTable = j;
				break;
			}
		}

		s_ServerClasses.push_back( entry );
	}


	for ( int i = 0; i < nServerClasses; i++ )
	{
		FlattenDataTable( i );
	}

	// perform integer log2() to set s_nServerClassBits
	int nTemp = nServerClasses;
	s_nServerClassBits = 0;
	while (nTemp >>= 1) ++s_nServerClassBits;

	s_nServerClassBits++;

	return true;
}

void FlattenDataTable( int nServerClass )
{
	CSVCMsg_SendTable *pTable = &s_DataTables[ s_ServerClasses[ nServerClass ].nDataTable ];

	s_currentExcludes.clear();
	GatherExcludes( pTable );

	GatherProps( pTable, nServerClass );

	std::vector< FlattenedPropEntry > &flattenedProps = s_ServerClasses[ nServerClass ].flattenedProps;

	// get priorities
	std::vector< uint32 > priorities;
	priorities.push_back(64);
	for ( unsigned int i = 0; i < flattenedProps.size(); i++ )
	{
		uint32 priority = flattenedProps[ i ].m_prop->priority();

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
				const CSVCMsg_SendTable::sendprop_t *prop = flattenedProps[currentProp].m_prop;

				if (prop->priority() == priority || (priority == 64 && (SPROP_CHANGES_OFTEN & prop->flags()))) 
				{
					if ( start != currentProp )
					{
						FlattenedPropEntry temp = flattenedProps[start];
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

void GatherExcludes( CSVCMsg_SendTable *pTable )
{
	for ( int iProp=0; iProp < pTable->props_size(); iProp++ )
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props( iProp );
		if ( sendProp.flags() & SPROP_EXCLUDE )
		{
			s_currentExcludes.push_back( ExcludeEntry( sendProp.var_name().c_str(), sendProp.dt_name().c_str(), pTable->net_table_name().c_str() ) );
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

void GatherProps( CSVCMsg_SendTable *pTable, int nServerClass )
{
	std::vector< FlattenedPropEntry > tempFlattenedProps;
	GatherProps_IterateProps( pTable, nServerClass, tempFlattenedProps );

	std::vector< FlattenedPropEntry > &flattenedProps = s_ServerClasses[ nServerClass ].flattenedProps;
	for ( uint32 i = 0; i < tempFlattenedProps.size(); i++ )
	{
		flattenedProps.push_back( tempFlattenedProps[ i ] );
	}
}

void GatherProps_IterateProps( CSVCMsg_SendTable *pTable, int nServerClass, std::vector< FlattenedPropEntry > &flattenedProps )
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
			if ( sendProp.type() == DPT_Array )
			{
				flattenedProps.push_back( FlattenedPropEntry( &sendProp, &(pTable->props( iProp - 1 ) ) ) );
			}
			else
			{
				flattenedProps.push_back( FlattenedPropEntry( &sendProp, NULL ) );
			}
		}
	}
}

bool IsPropExcluded( CSVCMsg_SendTable *pTable, const CSVCMsg_SendTable::sendprop_t &checkSendProp )
{
	for ( unsigned int i = 0; i < s_currentExcludes.size(); i++ )
	{
		if ( pTable->net_table_name().compare( s_currentExcludes[ i ].m_pDTName ) == 0 &&
			checkSendProp.var_name().compare( s_currentExcludes[ i ].m_pVarName ) == 0 )
		{
			return true;
		}
	}
	return false;
}

CSVCMsg_SendTable *GetTableByName( const char *pName )
{
	for ( unsigned int i = 0; i < s_DataTables.size(); i++ )
	{
		if ( s_DataTables[ i ].net_table_name().compare( pName ) == 0 )
		{
			return &(s_DataTables[ i ]);
		}
	}
	return NULL;
}
