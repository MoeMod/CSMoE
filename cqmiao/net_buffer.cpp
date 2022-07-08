/*
net_buffer.c - network bitbuffer io functions
Copyright (C) 2010 Uncle Mike
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "net_buffer.h"

#include <cmath>
#include <cstdio>

//#define DEBUG_NET_MESSAGES_SEND
//#define DEBUG_NET_MESSAGES_READ

// precalculated bit masks for WriteUBitLong.
// Using these tables instead of doing the calculations
// gives a 33% speedup in WriteUBitLong.
static thread_local std::uint32_t	BitWriteMasks[32][33];
static thread_local std::uint32_t	ExtraMasks[32];

#define BIT( n )		( 1U << ( n ))

short MSG_BigShort( short swap )
{
	return (swap >> 8)|(swap << 8);
}

void MSG_InitMasks( void )
{
	std::uint32_t	startbit, endbit;
	std::uint32_t	maskBit, nBitsLeft;

	for( startbit = 0; startbit < 32; startbit++ )
	{
		for( nBitsLeft = 0; nBitsLeft < 33; nBitsLeft++ )
		{
			endbit = startbit + nBitsLeft;

			BitWriteMasks[startbit][nBitsLeft] = (std::uint32_t)BIT( startbit ) - 1;
			if( endbit < 32 ) BitWriteMasks[startbit][nBitsLeft] |= ~((std::uint32_t)BIT( endbit ) - 1 );
		}
	}

	for( maskBit = 0; maskBit < 32; maskBit++ )
		ExtraMasks[maskBit] = (std::uint32_t)BIT( maskBit ) - 1;
}

static thread_local bool initialized = (MSG_InitMasks(), true);

void MSG_InitExt( sizebuf_t *sb, const char *pDebugName, void *pData, int nBytes, int nMaxBits )
{
	MSG_StartWriting( sb, pData, nBytes, 0, nMaxBits );

	sb->pDebugName = pDebugName;
}

void MSG_StartWriting( sizebuf_t *sb, void *pData, int nBytes, int iStartBit, int nBits )
{
	// make sure it's std::uint32_t aligned and padded.
	assert((reinterpret_cast<std::uintptr_t >(pData) & 3 ) == 0 );

	sb->pDebugName = "Unnamed";
	sb->pData = (std::uint8_t *)pData;

	if( nBits == -1 )
	{
		sb->nDataBits = nBytes << 3;
	}
	else
	{
		assert( nBits <= nBytes * 8 );
		sb->nDataBits = nBits;
	}

	sb->iCurBit = iStartBit;
	sb->bOverflow = false;
}

/*
=======================
MSG_Clear
for clearing overflowed buffer
=======================
*/
void MSG_Clear( sizebuf_t *sb )
{
	sb->iCurBit = 0;
	sb->bOverflow = false;
}

static bool MSG_Overflow( sizebuf_t *sb, int nBits )
{
	if( sb->iCurBit + nBits > sb->nDataBits )
		sb->bOverflow = true;
	return sb->bOverflow;
}

bool MSG_CheckOverflow( sizebuf_t *sb )
{
	return MSG_Overflow( sb, 0 );
}

int MSG_SeekToBit( sizebuf_t *sb, int bitPos, int whence )
{
	// compute the file offset
	switch( whence )
	{
		case SEEK_CUR:
			bitPos += sb->iCurBit;
			break;
		case SEEK_SET:
			break;
		case SEEK_END:
			bitPos += sb->nDataBits;
			break;
		default:
			return -1;
	}

	if( bitPos < 0 || bitPos > sb->nDataBits )
		return -1;

	sb->iCurBit = bitPos;

	return 0;
}

void MSG_SeekToByte( sizebuf_t *sb, int bytePos )
{
	sb->iCurBit = bytePos << 3;
}

void MSG_WriteOneBit( sizebuf_t *sb, int nValue )
{
	if( !MSG_Overflow( sb, 1 ))
	{
		if( nValue ) sb->pData[sb->iCurBit>>3] |= BIT( sb->iCurBit & 7 );
		else sb->pData[sb->iCurBit>>3] &= ~BIT( sb->iCurBit & 7 );

		sb->iCurBit++;
	}
}

void MSG_WriteUBitLong( sizebuf_t *sb, std::uint32_t curData, int numbits )
{
	assert( numbits >= 0 && numbits <= 32 );

	// bounds checking..
	if(( sb->iCurBit + numbits ) > sb->nDataBits )
	{
		sb->bOverflow = true;
		sb->iCurBit = sb->nDataBits;
	}
	else
	{
		int	nBitsLeft = numbits;
		int	iCurBit = sb->iCurBit;
		std::uint32_t	iDWord = iCurBit >> 5;	// Mask in a std::uint32_t.
		std::uint32_t	iCurBitMasked;
		int	nBitsWritten;

		assert(( iDWord * 4 + sizeof( int )) <= (std::uint32_t)MSG_GetMaxBytes( sb ));

		iCurBitMasked = iCurBit & 31;
		((std::uint32_t *)sb->pData)[iDWord] &= BitWriteMasks[iCurBitMasked][nBitsLeft];
		((std::uint32_t *)sb->pData)[iDWord] |= curData << iCurBitMasked;

		// did it span a std::uint32_t?
		nBitsWritten = 32 - iCurBitMasked;

		if( nBitsWritten < nBitsLeft )
		{
			nBitsLeft -= nBitsWritten;
			iCurBit += nBitsWritten;
			curData >>= nBitsWritten;

			iCurBitMasked = iCurBit & 31;
			((std::uint32_t *)sb->pData)[iDWord+1] &= BitWriteMasks[iCurBitMasked][nBitsLeft];
			((std::uint32_t *)sb->pData)[iDWord+1] |= curData << iCurBitMasked;
		}
		sb->iCurBit += numbits;
	}
}

/*
=======================
MSG_WriteSBitLong
sign bit comes first
=======================
*/
void MSG_WriteSBitLong( sizebuf_t *sb, int data, int numbits )
{
	// do we have a valid # of bits to encode with?
	assert( numbits >= 1 && numbits <= 32 );

	// NOTE: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	if( data < 0 )
	{
		MSG_WriteUBitLong( sb, (std::uint32_t)( 0x80000000 + data ), numbits - 1 );
		MSG_WriteOneBit( sb, 1 );
	}
	else
	{
		MSG_WriteUBitLong( sb, (std::uint32_t)data, numbits - 1 );
		MSG_WriteOneBit( sb, 0 );
	}
}

void MSG_WriteBitLong( sizebuf_t *sb, int data, int numbits, bool bSigned )
{
	if( bSigned )
		MSG_WriteSBitLong( sb, data, numbits );
	else MSG_WriteUBitLong( sb, (std::uint32_t)data, numbits );
}

bool MSG_WriteBits( sizebuf_t *sb, const void *pData, int nBits )
{
	std::uint8_t	*pOut = (std::uint8_t *)pData;
	int	nBitsLeft = nBits;

	// get output std::uint32_t-aligned.
	while((reinterpret_cast<std::uintptr_t>(pOut) & 3 ) != 0 && nBitsLeft >= 8 )
	{
		MSG_WriteUBitLong( sb, *pOut, 8 );

		nBitsLeft -= 8;
		++pOut;
	}

	// read std::uint32_ts.
	while( nBitsLeft >= 32 )
	{
		MSG_WriteUBitLong( sb, *(( std::uint32_t *)pOut ), 32 );

		pOut += sizeof( std::uint32_t );
		nBitsLeft -= 32;
	}

	// read the remaining std::uint8_ts.
	while( nBitsLeft >= 8 )
	{
		MSG_WriteUBitLong( sb, *pOut, 8 );

		nBitsLeft -= 8;
		++pOut;
	}

	// Read the remaining bits.
	if( nBitsLeft )
	{
		MSG_WriteUBitLong( sb, *pOut, nBitsLeft );
	}

	return !sb->bOverflow;
}

void MSG_WriteBitAngle( sizebuf_t *sb, float fAngle, int numbits )
{
	std::uint32_t	mask, shift;
	int	d;

	// clamp the angle before receiving
	fAngle = std::fmod( fAngle, 360.0f );
	if( fAngle < 0 ) fAngle += 360.0f;

	shift = ( 1 << numbits );
	mask = shift - 1;

	d = (int)(( fAngle * shift ) / 360.0f );
	d &= mask;

	MSG_WriteUBitLong( sb, (std::uint32_t)d, numbits );
}

void MSG_WriteCoord( sizebuf_t *sb, float val )
{
	MSG_WriteShort(sb, (int)(val * 8.0f));
}

void MSG_WriteVec3Coord( sizebuf_t *sb, const float *fa )
{
	MSG_WriteCoord( sb, fa[0] );
	MSG_WriteCoord( sb, fa[1] );
	MSG_WriteCoord( sb, fa[2] );
}

void MSG_WriteVec3Angles( sizebuf_t *sb, const float *fa )
{
	MSG_WriteBitAngle( sb, fa[0], 16 );
	MSG_WriteBitAngle( sb, fa[1], 16 );
	MSG_WriteBitAngle( sb, fa[2], 16 );
}

void MSG_WriteBitFloat( sizebuf_t *sb, float val )
{
	int	intVal;

	assert( sizeof( int ) == sizeof( float ));
	assert( sizeof( float ) == 4 );

	intVal = *((int *)&val );
	MSG_WriteUBitLong( sb, intVal, 32 );
}

void MSG_WriteChar( sizebuf_t *sb, int val )
{
	MSG_WriteSBitLong( sb, val, sizeof( char ) << 3 );
}

void MSG_WriteByte( sizebuf_t *sb, int val )
{
	MSG_WriteUBitLong( sb, val, sizeof( std::uint8_t ) << 3 );
}

void MSG_WriteShort( sizebuf_t *sb, int val )
{
	MSG_WriteSBitLong( sb, val, sizeof(short ) << 3 );
}

void MSG_WriteWord( sizebuf_t *sb, int val )
{
	MSG_WriteUBitLong( sb, val, sizeof( std::uint16_t ) << 3 );
}

void MSG_WriteLong( sizebuf_t *sb, int val )
{
	MSG_WriteSBitLong( sb, val, sizeof( int ) << 3 );
}

void MSG_WriteDword( sizebuf_t *sb, std::uint32_t val )
{
	MSG_WriteUBitLong( sb, val, sizeof( std::uint32_t ) << 3 );
}

void MSG_WriteFloat( sizebuf_t *sb, float val )
{
	MSG_WriteBits( sb, &val, sizeof( val ) << 3 );
}

bool MSG_WriteBytes( sizebuf_t *sb, const void *pBuf, int nBytes )
{
	return MSG_WriteBits( sb, pBuf, nBytes << 3 );
}

bool MSG_WriteString( sizebuf_t *sb, const char *pStr )
{
	if( pStr )
	{
		do
		{
			MSG_WriteChar( sb, *pStr );
			pStr++;
		} while( *( pStr - 1 ));
	}
	else MSG_WriteChar( sb, 0 );

	return !sb->bOverflow;
}

int MSG_ReadOneBit( sizebuf_t *sb )
{
	if( !MSG_Overflow( sb, 1 ))
	{
		int value = sb->pData[sb->iCurBit >> 3] & (1 << ( sb->iCurBit & 7 ));
		sb->iCurBit++;
		return !!value;
	}
	return 0;
}

std::uint32_t MSG_ReadUBitLong( sizebuf_t *sb, int numbits )
{
	int	idword1;
	std::uint32_t	dword1, ret;

	if (numbits == 8)
	{
		int leftBits = MSG_GetNumBitsLeft(sb);

		if (leftBits >= 0 && leftBits < 8)
			return 0;	// end of message
	}

	if ((sb->iCurBit + numbits) > sb->nDataBits)
	{
		sb->bOverflow = true;
		sb->iCurBit = sb->nDataBits;
		return 0;
	}

	assert(numbits > 0 && numbits <= 32);

	// Read the current dword.
	idword1 = sb->iCurBit >> 5;
	dword1 = ((std::uint32_t*)sb->pData)[idword1];
	dword1 >>= (sb->iCurBit & 31);	// get the bits we're interested in.

	sb->iCurBit += numbits;
	ret = dword1;

	// Does it span this dword?
	if ((sb->iCurBit - 1) >> 5 == idword1)
	{
		if (numbits != 32)
			ret &= ExtraMasks[numbits];
	}
	else
	{
		int	nExtraBits = sb->iCurBit & 31;
		std::uint32_t	dword2 = ((std::uint32_t*)sb->pData)[idword1 + 1] & ExtraMasks[nExtraBits];

		// no need to mask since we hit the end of the dword.
		// shift the second dword's part into the high bits.
		ret |= (dword2 << (numbits - nExtraBits));
	}
	return ret;
}

float MSG_ReadBitFloat( sizebuf_t *sb )
{
	int	val;
	int	bit, byte;

	assert( sizeof( float ) == sizeof( int ));
	assert( sizeof( float ) == 4 );

	if( MSG_Overflow( sb, 32 ))
		return 0.0f;

	bit = sb->iCurBit & 0x7;
	byte = sb->iCurBit >> 3;

	val = sb->pData[byte] >> bit;
	val |= ((int)sb->pData[byte + 1]) << ( 8 - bit );
	val |= ((int)sb->pData[byte + 2]) << ( 16 - bit );
	val |= ((int)sb->pData[byte + 3]) << ( 24 - bit );

	if( bit != 0 )
		val |= ((int)sb->pData[byte + 4]) << ( 32 - bit );
	sb->iCurBit += 32;

	return *((float *)&val);
}

bool MSG_ReadBits( sizebuf_t *sb, void *pOutData, int nBits )
{
	std::uint8_t	*pOut = (std::uint8_t *)pOutData;
	int	nBitsLeft = nBits;

	// get output std::uint32_t-aligned.
	while((reinterpret_cast<std::uintptr_t>(pOut) & 3) != 0 && nBitsLeft >= 8 )
	{
		*pOut = (std::uint8_t)MSG_ReadUBitLong( sb, 8 );
		++pOut;
		nBitsLeft -= 8;
	}

	// read std::uint32_ts.
	while( nBitsLeft >= 32 )
	{
		*((std::uint32_t *)pOut) = MSG_ReadUBitLong( sb, 32 );
		pOut += sizeof( std::uint32_t );
		nBitsLeft -= 32;
	}

	// read the remaining std::uint8_ts.
	while( nBitsLeft >= 8 )
	{
		*pOut = MSG_ReadUBitLong( sb, 8 );
		++pOut;
		nBitsLeft -= 8;
	}

	// read the remaining bits.
	if( nBitsLeft )
	{
		*pOut = MSG_ReadUBitLong( sb, nBitsLeft );
	}

	return !sb->bOverflow;
}

float MSG_ReadBitAngle( sizebuf_t *sb, int numbits )
{
	float	fReturn, shift;
	int	i;

	shift = (float)( 1 << numbits );

	i = MSG_ReadUBitLong( sb, numbits );
	fReturn = (float)i * ( 360.0f / shift );

	// clamp the finale angle
	if( fReturn < -180.0f ) fReturn += 360.0f;
	else if( fReturn > 180.0f ) fReturn -= 360.0f;

	return fReturn;
}

// Append numbits least significant bits from data to the current bit stream
int MSG_ReadSBitLong( sizebuf_t *sb, int numbits )
{
	int	r, sign;

	r = MSG_ReadUBitLong( sb, numbits - 1 );

	// NOTE: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	sign = MSG_ReadOneBit( sb );
	if( sign ) r = -( (int)BIT( numbits - 1 ) - r );

	return r;
}

std::uint32_t MSG_ReadBitLong( sizebuf_t *sb, int numbits, bool bSigned )
{
	if( bSigned )
		return (std::uint32_t)MSG_ReadSBitLong( sb, numbits );
	return MSG_ReadUBitLong( sb, numbits );
}

int MSG_ReadChar( sizebuf_t *sb )
{
	return MSG_ReadSBitLong( sb, sizeof( char ) << 3 );
}

int MSG_ReadByte( sizebuf_t *sb )
{
	return MSG_ReadUBitLong( sb, sizeof( std::uint8_t ) << 3 );
}

int MSG_ReadShort( sizebuf_t *sb )
{
	return MSG_ReadSBitLong( sb, sizeof( short ) << 3 );
}

int MSG_ReadWord( sizebuf_t *sb )
{
	return MSG_ReadUBitLong( sb, sizeof( std::uint16_t ) << 3 );
}

float MSG_ReadCoord( sizebuf_t *sb )
{
	return (float)(MSG_ReadShort( sb ) * ( 1.0f / 8.0f ));
}

void MSG_ReadVec3Coord( sizebuf_t *sb, float fa[3] )
{
	fa[0] = MSG_ReadCoord( sb );
	fa[1] = MSG_ReadCoord( sb );
	fa[2] = MSG_ReadCoord( sb );
}

void MSG_ReadVec3Angles( sizebuf_t *sb, float fa[3])
{
	fa[0] = MSG_ReadBitAngle( sb, 16 );
	fa[1] = MSG_ReadBitAngle( sb, 16 );
	fa[2] = MSG_ReadBitAngle( sb, 16 );
}

int MSG_ReadLong( sizebuf_t *sb )
{
	return MSG_ReadSBitLong( sb, sizeof( int ) << 3 );
}

std::uint32_t MSG_ReadDword( sizebuf_t *sb )
{
	return MSG_ReadUBitLong( sb, sizeof( std::uint32_t ) << 3 );
}

float MSG_ReadFloat( sizebuf_t *sb )
{
	float	ret;

	assert( sizeof( ret ) == 4 );

	MSG_ReadBits( sb, &ret, 32 );

	return ret;
}

bool MSG_ReadBytes( sizebuf_t *sb, void *pOut, int nBytes )
{
	return MSG_ReadBits( sb, pOut, nBytes << 3 );
}

char *MSG_ReadStringExt( sizebuf_t *sb, bool bLine )
{
	static char	string[4096];
	int		l = 0, c;

	do
	{
		// use MSG_ReadByte so -1 is out of bounds
		c = MSG_ReadByte( sb );

		if( c == 0 ) break;
		else if( bLine && c == '\n' )
			break;

		// translate all fmt spec to avoid crash bugs
		// NOTE: but game strings leave unchanged. see pfnWriteString for details
		if( c == '%' ) c = '.';

		string[l] = c;
		l++;
	} while( l < sizeof( string ) - 1 );
	string[l] = 0; // terminator

	return string;
}

void MSG_ExciseBits( sizebuf_t *sb, int startbit, int bitstoremove )
{
	int	i, endbit = startbit + bitstoremove;
	int	remaining_to_end = sb->nDataBits - endbit;
	sizebuf_t	temp;

	MSG_StartWriting( &temp, sb->pData, MSG_GetMaxBytes( sb ), startbit, -1 );
	MSG_SeekToBit( sb, endbit, SEEK_SET );

	for( i = 0; i < remaining_to_end; i++ )
	{
		MSG_WriteOneBit( &temp, MSG_ReadOneBit( sb ));
	}

	MSG_SeekToBit( sb, startbit, SEEK_SET );
	sb->nDataBits -= bitstoremove;
}