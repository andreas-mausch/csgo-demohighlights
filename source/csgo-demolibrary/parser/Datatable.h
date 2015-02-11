#pragma once

#include "../sdk/demofile.h"

class CSVCMsg_SendTable;
class MemoryBitStream;
struct FlattenedPropEntry;

bool ParseDataTable( MemoryBitStream &buf );

CSVCMsg_SendTable *GetTableByClassID( uint32 nClassID );
FlattenedPropEntry *GetSendPropByIndex( uint32 uClass, uint32 uIndex );
