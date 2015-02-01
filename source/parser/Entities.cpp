#include "Datatable.h"
#include "Entities.h"

#include "../streams/MemoryStream.h"
#include "../streams/MemoryBitStream.h"
#include "../streams/MemoryStreamBuffer.h"

#include "../sdk/demofile.h"
#include "../sdk/demofiledump.h"
#include "../protobuf/generated/netmessages_public.pb.h"

std::vector< EntityEntry * > s_Entities;
extern int s_nServerClassBits;

void packetEntities(CSVCMsg_PacketEntities &message);
EntityEntry *FindEntity( int nEntity );
EntityEntry *AddEntity( int nEntity, uint32 uClass, uint32 uSerialNum );
void RemoveEntity( int nEntity);
bool ReadNewEntity( MemoryBitStream &entityBitBuffer, EntityEntry *pEntity );
int ReadFieldIndex( MemoryBitStream &entityBitBuffer, int lastIndex, bool bNewWay );
Prop_t *DecodeProp( MemoryBitStream &entityBitBuffer, FlattenedPropEntry *pFlattenedProp, uint32 uClass, int nFieldIndex, bool bQuiet );

float ReadBitCoord(MemoryBitStream &stream);
float ReadBitCoordMP(MemoryBitStream &stream, EBitCoordType coordType);
float ReadBitCellCoord(MemoryBitStream &stream, int bits, EBitCoordType coordType);
float ReadBitNormal(MemoryBitStream &stream);
float ReadBitFloat(MemoryBitStream &stream);
int64 ReadSignedVarInt64(MemoryBitStream &stream);

void packetEntities(CSVCMsg_PacketEntities &message)
{
	MemoryStreamBuffer buffer(message.entity_data().c_str(), message.entity_data().size());
	MemoryBitStream stream(buffer);

	bool bAsDelta = message.is_delta();
	int nHeaderCount = message.updated_entries();
	int nBaseline = message.baseline();
	bool bUpdateBaselines = message.update_baseline();
	int nHeaderBase = -1;
	int nNewEntity = -1;
	int UpdateFlags = 0;

	UpdateType updateType = PreserveEnt;

	while ( updateType < Finished )
	{
		nHeaderCount--;

		bool bIsEntity = ( nHeaderCount >= 0 ) ? true : false;

		if ( bIsEntity  )
		{
			UpdateFlags = FHDR_ZERO;

			nNewEntity = nHeaderBase + 1 + stream.ReadUBitVar();
			nHeaderBase = nNewEntity;

			// leave pvs flag
			if (!stream.readBit())
			{
				// enter pvs flag
				if (stream.readBit())
				{
					UpdateFlags |= FHDR_ENTERPVS;
				}
			}
			else
			{
				UpdateFlags |= FHDR_LEAVEPVS;

				// Force delete flag
				if (stream.readBit())
				{
					UpdateFlags |= FHDR_DELETE;
				}
			}
		}

		for ( updateType = PreserveEnt; updateType == PreserveEnt; )
		{
			// Figure out what kind of an update this is.
			if ( !bIsEntity || nNewEntity > ENTITY_SENTINEL)
			{
				updateType = Finished;
			}
			else
			{
				if ( UpdateFlags & FHDR_ENTERPVS )
				{
					updateType = EnterPVS;
				}
				else if ( UpdateFlags & FHDR_LEAVEPVS )
				{
					updateType = LeavePVS;
				}
				else
				{
					updateType = DeltaEnt;
				}
			}

			switch( updateType )
			{
			case EnterPVS:	
				{
					uint32 uClass = stream.ReadUBitLong( s_nServerClassBits );
					uint32 uSerialNum = stream.ReadUBitLong( NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS );
					EntityEntry *pEntity = AddEntity( nNewEntity, uClass, uSerialNum );
					if ( !ReadNewEntity( stream, pEntity ) )
					{
						printf( "*****Error reading entity! Bailing on this PacketEntities!\n" );
						return;
					}
				}
				break;

			case LeavePVS:
				{
					if ( !bAsDelta )  // Should never happen on a full update.
					{
						printf( "WARNING: LeavePVS on full update" );
						updateType = Failed;	// break out
						assert( 0 );
					}
					else
					{
						RemoveEntity( nNewEntity );
					}
				}
				break;

			case DeltaEnt:
				{
					EntityEntry *pEntity = FindEntity( nNewEntity );
					if ( pEntity )
					{
						if ( !ReadNewEntity( stream, pEntity ) )
						{
							printf( "*****Error reading entity! Bailing on this PacketEntities!\n" );
							return;
						}
					}
					else
					{
						assert(0);
					}
				}
				break;

			case PreserveEnt:
				{
					if ( !bAsDelta )  // Should never happen on a full update.
					{
						printf( "WARNING: PreserveEnt on full update" );
						updateType = Failed;	// break out
						assert( 0 );
					}
					else
					{
						if ( nNewEntity >= MAX_EDICTS )
						{
							printf( "PreserveEnt: nNewEntity == MAX_EDICTS" );
							assert( 0 );
						}
						else
						{
						}
					}
				}
				break;

			default:
				break;
			}
		}
	}
}

EntityEntry *FindEntity( int nEntity )
{
	for ( std::vector< EntityEntry * >::iterator i = s_Entities.begin(); i != s_Entities.end(); i++ )
	{
		if (  (*i)->m_nEntity == nEntity )
		{
			return *i;
		}
	}

	return NULL;
}

EntityEntry *AddEntity( int nEntity, uint32 uClass, uint32 uSerialNum )
{
	// if entity already exists, then replace it, else add it
	EntityEntry *pEntity = FindEntity( nEntity );
	if ( pEntity )
	{
		pEntity->m_uClass = uClass;
		pEntity->m_uSerialNum = uSerialNum;
	}
	else
	{
		pEntity = new EntityEntry( nEntity, uClass, uSerialNum );
		s_Entities.push_back( pEntity );
	}

	return pEntity;
}

void RemoveEntity( int nEntity )
{
	for ( std::vector< EntityEntry * >::iterator i = s_Entities.begin(); i != s_Entities.end(); i++ )
	{
		EntityEntry *pEntity = *i;
		if (  pEntity->m_nEntity == nEntity )
		{
			s_Entities.erase( i );
			delete pEntity;
			break;
		}
	}
}

bool ReadNewEntity( MemoryBitStream &entityBitBuffer, EntityEntry *pEntity )
{
	bool bNewWay = entityBitBuffer.readBit();

	std::vector< int > fieldIndices;

	int index = -1;
	do
	{
		index = ReadFieldIndex( entityBitBuffer, index, bNewWay );
		if ( index != -1 )
		{
			fieldIndices.push_back( index );
		}
	} while (index != -1);

	CSVCMsg_SendTable *pTable = GetTableByClassID( pEntity->m_uClass );
	for ( unsigned int i = 0; i < fieldIndices.size(); i++ )
	{
		FlattenedPropEntry *pSendProp = GetSendPropByIndex( pEntity->m_uClass, fieldIndices[ i ] );
		if ( pSendProp )
		{
			Prop_t *pProp = DecodeProp( entityBitBuffer, pSendProp, pEntity->m_uClass, fieldIndices[ i ], true);
			pEntity->AddOrUpdateProp( pSendProp, pProp );
		}
		else
		{
			return false;
		}
	}

	return true;
}

int ReadFieldIndex( MemoryBitStream &entityBitBuffer, int lastIndex, bool bNewWay )
{
	if (bNewWay)
	{
		if (entityBitBuffer.readBit())
		{
			return lastIndex + 1;
		}
	}
 
	int ret = 0;
	if (bNewWay && entityBitBuffer.readBit())
	{
		ret = entityBitBuffer.ReadUBitLong(3);  // read 3 bits
	}
	else
	{
		ret = entityBitBuffer.ReadUBitLong(7); // read 7 bits
		switch( ret & ( 32 | 64 ) )
		{
			case 32:
				ret = ( ret &~96 ) | ( entityBitBuffer.ReadUBitLong( 2 ) << 5 );
				assert( ret >= 32);
				break;
			case 64:
				ret = ( ret &~96 ) | ( entityBitBuffer.ReadUBitLong( 4 ) << 5 );
				assert( ret >= 128);
				break;
			case 96:
				ret = ( ret &~96 ) | ( entityBitBuffer.ReadUBitLong( 7 ) << 5 );
				assert( ret >= 512);
				break;
		}
	}
 
	if (ret == 0xFFF) // end marker is 4095 for cs:go
	{
		return -1;
	}
 
	return lastIndex + 1 + ret;
}

int Int_Decode( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp )
{
	int flags = pSendProp->flags();

	if ( flags & SPROP_VARINT )
	{
		return entityBitBuffer.readVarInt32();
	}
	else
	{
		if ( flags & SPROP_UNSIGNED )
		{
			return entityBitBuffer.ReadUBitLong( pSendProp->num_bits() );
		}
		else
		{
			return entityBitBuffer.ReadSBitLong( pSendProp->num_bits() );
		}
	}
}

// Look for special flags like SPROP_COORD, SPROP_NOSCALE, and SPROP_NORMAL and
// decode if they're there. Fills in fVal and returns true if it decodes anything.
static inline bool DecodeSpecialFloat( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp, float &fVal )
{
	int flags = pSendProp->flags();

	if ( flags & SPROP_COORD )
	{
		fVal = ReadBitCoord(entityBitBuffer);
		return true;
	}
	else if ( flags & SPROP_COORD_MP )
	{
		fVal = ReadBitCoordMP(entityBitBuffer, kCW_None);
		return true;
	}
	else if ( flags & SPROP_COORD_MP_LOWPRECISION )
	{
		fVal = ReadBitCoordMP(entityBitBuffer, kCW_LowPrecision);
		return true;
	}
	else if ( flags & SPROP_COORD_MP_INTEGRAL )
	{
		fVal = ReadBitCoordMP(entityBitBuffer, kCW_Integral);
		return true;
	}
	else if ( flags & SPROP_NOSCALE )
	{
		fVal = ReadBitFloat(entityBitBuffer);
		return true;
	}
	else if ( flags & SPROP_NORMAL )
	{
		fVal = ReadBitNormal(entityBitBuffer);
		return true;
	}
	else if ( flags & SPROP_CELL_COORD )
	{
		fVal = ReadBitCellCoord(entityBitBuffer, pSendProp->num_bits(), kCW_None );
		return true;
	}
	else if ( flags & SPROP_CELL_COORD_LOWPRECISION )
	{
		fVal = ReadBitCellCoord(entityBitBuffer, pSendProp->num_bits(), kCW_LowPrecision );
		return true;
	}
	else if ( flags & SPROP_CELL_COORD_INTEGRAL )
	{
		fVal = ReadBitCellCoord(entityBitBuffer, pSendProp->num_bits(), kCW_Integral );
		return true;
	}

	return false;
}

float Float_Decode( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp )
{
	float fVal = 0.0f;
	unsigned long dwInterp;

	// Check for special flags..
	if( DecodeSpecialFloat( entityBitBuffer, pSendProp, fVal ) )
	{
		return fVal;
	}

	dwInterp = entityBitBuffer.ReadUBitLong( pSendProp->num_bits() );
	fVal = ( float )dwInterp / ( ( 1 << pSendProp->num_bits() ) - 1 );
	fVal = pSendProp->low_value() + (pSendProp->high_value() - pSendProp->low_value()) * fVal;
	return fVal;
}

void Vector_Decode( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp, Vector &v )
{
	v.x = Float_Decode( entityBitBuffer, pSendProp );
	v.y = Float_Decode( entityBitBuffer, pSendProp );

	// Don't read in the third component for normals
	if ( ( pSendProp->flags() & SPROP_NORMAL ) == 0 )
	{
		v.z = Float_Decode( entityBitBuffer, pSendProp );
	}
	else
	{
		int signbit = entityBitBuffer.readBit();

		float v0v0v1v1 = v.x * v.x + v.y * v.y;
		if (v0v0v1v1 < 1.0f)
		{
			v.z = sqrtf( 1.0f - v0v0v1v1 );
		}
		else
		{
			v.z = 0.0f;
		}

		if (signbit)
		{
			v.z *= -1.0f;
		}
	}
}

void VectorXY_Decode( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp, Vector &v )
{
	v.x = Float_Decode( entityBitBuffer, pSendProp );
	v.y = Float_Decode( entityBitBuffer, pSendProp );
}

const char *String_Decode( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp )
{
	// Read it in.
	int len = entityBitBuffer.ReadUBitLong( DT_MAX_STRING_BITS );

	char *tempStr = new char[ len + 1 ];

	if ( len >= DT_MAX_STRING_BUFFERSIZE )
	{
		printf( "String_Decode( %s ) invalid length (%d)\n", pSendProp->var_name().c_str(), len );
		len = DT_MAX_STRING_BUFFERSIZE - 1;
	}

	entityBitBuffer.readBits( tempStr, len*8 );
	tempStr[len] = 0;

	return tempStr;
}

int64 Int64_Decode( MemoryBitStream &entityBitBuffer, const CSVCMsg_SendTable::sendprop_t *pSendProp )
{
	if ( pSendProp->flags() & SPROP_VARINT )
	{
		return ReadSignedVarInt64(entityBitBuffer);
	}
	else
	{
		uint32 highInt = 0;
		uint32 lowInt = 0;
		bool bNeg = false;
		if( !(pSendProp->flags() & SPROP_UNSIGNED) )
		{
			bNeg = entityBitBuffer.readBit();
			lowInt = entityBitBuffer.ReadUBitLong( 32 );
			highInt = entityBitBuffer.ReadUBitLong( pSendProp->num_bits() - 32 - 1 );
		}
		else
		{
			lowInt = entityBitBuffer.ReadUBitLong( 32 );
			highInt = entityBitBuffer.ReadUBitLong( pSendProp->num_bits() - 32 );
		}

		int64 temp;

		uint32 *pInt = (uint32*)&temp;
		*pInt++ = lowInt;
		*pInt = highInt;

		if ( bNeg )
		{
			temp = -temp;
		}

		return temp;
	}
}

Prop_t *Array_Decode( MemoryBitStream &entityBitBuffer, FlattenedPropEntry *pFlattenedProp, int nNumElements, uint32 uClass, int nFieldIndex, bool bQuiet )
{
	int maxElements = nNumElements;
	int numBits = 1;
	while ( (maxElements >>= 1) != 0 )
	{
		numBits++;
	}

	int nElements = entityBitBuffer.ReadUBitLong( numBits );

	Prop_t *pResult = NULL;
	pResult = new Prop_t[ nElements ];

	if ( !bQuiet )
	{
		printf( "array with %d elements of %d max\n", nElements, nNumElements );
	}

	for ( int i = 0; i < nElements; i++ )
	{
		FlattenedPropEntry temp( pFlattenedProp->m_arrayElementProp, NULL );
		Prop_t *pElementResult = DecodeProp( entityBitBuffer, &temp, uClass, nFieldIndex, bQuiet );
		pResult[ i ] = *pElementResult;
		delete pElementResult;
		pResult[ i ].m_nNumElements = nElements - i;
	}

	return pResult;
}

Prop_t *DecodeProp( MemoryBitStream &entityBitBuffer, FlattenedPropEntry *pFlattenedProp, uint32 uClass, int nFieldIndex, bool bQuiet )
{
	const CSVCMsg_SendTable::sendprop_t *pSendProp = pFlattenedProp->m_prop;

	Prop_t *pResult = NULL;
	if ( pSendProp->type() != DPT_Array && pSendProp->type() != DPT_DataTable )
	{
		pResult = new Prop_t( ( SendPropType_t )( pSendProp->type() ) );
	}

	if ( !bQuiet )
	{
		printf( "Field: %d, %s = ", nFieldIndex, pSendProp->var_name().c_str() );
	}
	switch ( pSendProp->type() )
	{
		case DPT_Int:
			pResult->m_value.m_int = Int_Decode( entityBitBuffer, pSendProp );
			break;
		case DPT_Float:
			pResult->m_value.m_float = Float_Decode( entityBitBuffer, pSendProp );
			break;
		case DPT_Vector:
			Vector_Decode( entityBitBuffer, pSendProp, pResult->m_value.m_vector );
			break;
		case DPT_VectorXY:
			VectorXY_Decode( entityBitBuffer, pSendProp, pResult->m_value.m_vector );
			break;
		case DPT_String:
			pResult->m_value.m_pString = String_Decode( entityBitBuffer, pSendProp );
			break;
		case DPT_Array:
			pResult = Array_Decode( entityBitBuffer, pFlattenedProp, pSendProp->num_elements(), uClass, nFieldIndex, bQuiet );
			break;
		case DPT_DataTable:
			break;
		case DPT_Int64:
			pResult->m_value.m_int64 = Int64_Decode( entityBitBuffer, pSendProp );
			break;
	}

	return pResult;
}

float ReadBitCoord (MemoryBitStream &stream)
{
	int		intval=0,fractval=0,signbit=0;
	float	value = 0.0;


	// Read the required integer and fraction flags
	intval = stream.readBit();
	fractval = stream.readBit();

	// If we got either parse them, otherwise it's a zero.
	if ( intval || fractval )
	{
		// Read the sign bit
		signbit = stream.readBit();

		// If there's an integer, read it in
		if ( intval )
		{
			// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
			intval = stream.ReadUBitLong( COORD_INTEGER_BITS ) + 1;
		}

		// If there's a fraction, read it in
		if ( fractval )
		{
			fractval = stream.ReadUBitLong( COORD_FRACTIONAL_BITS );
		}

		// Calculate the correct floating point value
		value = intval + ((float)fractval * COORD_RESOLUTION);

		// Fixup the sign if negative.
		if ( signbit )
			value = -value;
	}

	return value;
}

float ReadBitCoordMP(MemoryBitStream &stream, EBitCoordType coordType )
{
	bool bIntegral = ( coordType == kCW_Integral );
	bool bLowPrecision = ( coordType == kCW_LowPrecision );  

	int		intval=0,fractval=0,signbit=0;
	float	value = 0.0;

	bool bInBounds = stream.readBit();

	if ( bIntegral )
	{
		// Read the required integer and fraction flags
		intval = stream.readBit();
		// If we got either parse them, otherwise it's a zero.
		if ( intval )
		{
			// Read the sign bit
			signbit = stream.readBit();

			// If there's an integer, read it in
			// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
			if ( bInBounds )
			{
				value = ( float )( stream.ReadUBitLong( COORD_INTEGER_BITS_MP ) + 1 );
			}
			else
			{
				value = ( float )( stream.ReadUBitLong( COORD_INTEGER_BITS ) + 1 );
			}
		}
	}
	else
	{
		// Read the required integer and fraction flags
		intval = stream.readBit();

		// Read the sign bit
		signbit = stream.readBit();

		// If we got either parse them, otherwise it's a zero.
		if ( intval )
		{
			if ( bInBounds )
			{
				intval = stream.ReadUBitLong( COORD_INTEGER_BITS_MP ) + 1;
			}
			else
			{
				intval = stream.ReadUBitLong( COORD_INTEGER_BITS ) + 1;
			}
		}

		// If there's a fraction, read it in
		fractval = stream.ReadUBitLong( bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS );

		// Calculate the correct floating point value
		value = intval + ((float)fractval * ( bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION ) );
	}

	// Fixup the sign if negative.
	if ( signbit )
		value = -value;

	return value;
}

float ReadBitCellCoord(MemoryBitStream &stream, int bits, EBitCoordType coordType )
{
	bool bIntegral = ( coordType == kCW_Integral );
	bool bLowPrecision = ( coordType == kCW_LowPrecision );  

	int		intval=0,fractval=0;
	float	value = 0.0;

	if ( bIntegral )
	{
		value = ( float )( stream.ReadUBitLong( bits ) );
	}
	else
	{
		intval = stream.ReadUBitLong( bits );

		// If there's a fraction, read it in
		fractval = stream.ReadUBitLong( bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS );

		// Calculate the correct floating point value
		value = intval + ((float)fractval * ( bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION ) );
	}

	return value;
}

float ReadBitNormal (MemoryBitStream &stream)
{
	// Read the sign bit
	int	signbit = stream.readBit();

	// Read the fractional part
	unsigned int fractval = stream.ReadUBitLong( NORMAL_FRACTIONAL_BITS );

	// Calculate the correct floating point value
	float value = (float)fractval * NORMAL_RESOLUTION;

	// Fixup the sign if negative.
	if ( signbit )
		value = -value;

	return value;
}

float ReadBitFloat( MemoryBitStream &stream )
{
	uint32 nvalue = stream.ReadUBitLong( 32 );
	return *( ( float * ) &nvalue );
}

inline int64 ZigZagDecode64(uint64 n) 
{
	return(n >> 1) ^ -static_cast<int64>(n & 1);
}

uint64 ReadVarInt64(MemoryBitStream &stream)
{
	const int kMaxVarintBytes = 10;
	uint64 result = 0;
	int count = 0;
	uint64 b;

	do 
	{
		if ( count == kMaxVarintBytes ) 
		{
			return result;
		}
		b = stream.ReadUBitLong( 8 );
		result |= static_cast<uint64>(b & 0x7F) << (7 * count);
		++count;
	} while (b & 0x80);

	return result;
}

int64 ReadSignedVarInt64(MemoryBitStream &stream) { return ZigZagDecode64( ReadVarInt64(stream) ); }
