//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef COMMONMACROS_H
#define COMMONMACROS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

// -------------------------------------------------------
//
// commonmacros.h
//
// This should contain ONLY general purpose macros that are 
// appropriate for use in engine/launcher/all tools
//
// -------------------------------------------------------

// Makes a 4-byte "packed ID" int out of 4 characters
#define MAKEID(d,c,b,a)					( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )

// Compares a string with a 4-byte packed ID constant
#define STRING_MATCHES_ID( p, id )		( (*((int *)(p)) == (id) ) ? true : false )
#define ID_TO_STRING( id, p )			( (p)[3] = (((id)>>24) & 0xFF), (p)[2] = (((id)>>16) & 0xFF), (p)[1] = (((id)>>8) & 0xFF), (p)[0] = (((id)>>0) & 0xFF) )

#define SETBITS(iBitVector, bits)		((iBitVector) |= (bits))
#define CLEARBITS(iBitVector, bits)		((iBitVector) &= ~(bits))
#define FBITSET(iBitVector, bits)		((iBitVector) & (bits))

template <typename T>
inline bool IsPowerOfTwo( T value )
{
	return (value & ( value - (T)1 )) == (T)0;
}

#ifndef REFERENCE
#define REFERENCE(arg) ((void)arg)
#endif

#define CONST_INTEGER_AS_STRING(x) #x //Wraps the integer in quotes, allowing us to form constant strings with it
#define __HACK_LINE_AS_STRING__(x) CONST_INTEGER_AS_STRING(x) //__LINE__ can only be converted to an actual number by going through this, otherwise the output is literally "__LINE__"
#define __LINE__AS_STRING __HACK_LINE_AS_STRING__(__LINE__) //Gives you the line number in constant string form

#include "arraysize.h"

#define Q_ARRAYSIZE(p)		ARRAYSIZE(p)
#define V_ARRAYSIZE(p)		ARRAYSIZE(p)

template< typename IndexType, typename T, unsigned int N >
IndexType ClampedArrayIndex( const T (&buffer)[N], IndexType index )
{
	NOTE_UNUSED( buffer );
	return clamp( index, 0, (IndexType)N - 1 );
}

template< typename T, unsigned int N >
T ClampedArrayElement( const T (&buffer)[N], unsigned int uIndex )
{
	// Put index in an unsigned type to halve the clamping.
	if ( uIndex >= N )
		uIndex = N - 1;
	return buffer[ uIndex ];
}

#endif // COMMONMACROS_H
