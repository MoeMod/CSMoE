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

// Quake string (helper class)
class QString final
{
public:
	using qstring_t = int;

	constexpr QString() noexcept : m_string(0) {};
	constexpr QString(qstring_t string) noexcept : m_string(string) {};

	constexpr bool IsNull() const noexcept
	{
		return m_string == 0;
	}
	bool IsNullOrEmpty() const noexcept;

	// Copy the array
	//QString &operator=(const QString &other) noexcept = default;

	constexpr bool operator==(qstring_t string) const noexcept
	{
		return m_string == string;
	}
	constexpr bool operator==(const QString &s) const noexcept
	{
		return m_string == s.m_string;
	}
	DEPRECATED bool operator==(const char *pszString) const noexcept;

	DEPRECATED operator const char *() const noexcept
	{
		return str();
	}
	constexpr operator qstring_t() const noexcept
	{
		return m_string;
	}
	const char *str() const noexcept;

private:
	qstring_t m_string;
};

constexpr QString iStringNull {};	// Testing strings for nullity

#ifdef USE_QSTRING
#define string_t QString
#endif

#include "const.h"
#include "edict.h"
#include "enginecallback.h"

#include "stddef.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

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

inline bool QString::IsNullOrEmpty() const noexcept
{
	return IsNull() || (&gpGlobals->pStringBase[m_string])[0] == '\0';
}

inline bool QString::operator==(const char *pszString) const noexcept
{
	return Q_strcmp(str(), pszString) == 0;
}

inline const char *QString::str() const noexcept
{
	return &gpGlobals->pStringBase[m_string];
}
