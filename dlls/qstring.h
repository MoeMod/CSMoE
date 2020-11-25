/*
qstring.h
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

constexpr string_t iStringNull{};

//#define STRING(offset)   ((const char *)(gpGlobals->pStringBase + (ptrdiff_t)(offset)))
inline const char *STRING(string_t offset)
{
	return gpGlobals->pStringBase + (ptrdiff_t)(offset);
}

#if !defined XASH_64BIT || defined(CLIENT_DLL)
//#define MAKE_STRING(str)	((int)(long int)str - (int)(long int)STRING(0))
inline string_t MAKE_STRING(const char *szValue)
{
	ptrdiff_t ptrdiff = szValue - STRING(0);
	return (int)ptrdiff;
}
#else
inline string_t MAKE_STRING(const char *szValue)
{
	ptrdiff_t ptrdiff = szValue - STRING(0);
	if (ptrdiff > INT_MAX || ptrdiff < INT_MIN)
		return ALLOC_STRING(szValue);
	else
		return (int)ptrdiff;
}
#endif

}
