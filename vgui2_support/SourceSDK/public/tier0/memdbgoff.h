//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: This header, which must be the final line of a .h file,
// causes all crt methods to stop using debugging versions of the memory allocators.
// NOTE: Use memdbgon.h to re-enable memory debugging.
//
// $NoKeywords: $
//=============================================================================//

#ifdef MEM_OVERRIDE_ON

#undef MEM_OVERRIDE_ON

#endif
