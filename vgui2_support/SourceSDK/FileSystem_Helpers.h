//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FILESYSTEM_HELPERS_H
#define FILESYSTEM_HELPERS_H
#ifdef _WIN32
#pragma once
#endif

class IFileSystem;

// Call until it returns NULL. Each time you call it, it will parse out a token.
typedef struct characterset_s characterset_t;
const char* ParseFile( const char* pFileBytes, char* pToken, bool* pWasQuoted, characterset_t *pCharSet = NULL );
char* ParseFile( char* pFileBytes, char* pToken, bool* pWasQuoted );	// (same exact thing as the const version)

bool FS_GetFileTypeForFullPath( char const *pFullPath, wchar_t *buf, size_t bufSizeInBytes );

bool FS_IsFileWritable( IFileSystem* pFileSystem, char const *pFileName, const char *pPathID = 0 );


#endif // FILESYSTEM_HELPERS_H
