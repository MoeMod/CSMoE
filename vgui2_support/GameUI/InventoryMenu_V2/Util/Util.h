#pragma once

typedef struct CsoWpnname_s
{
	const char* source;
	const char* dest;
} CsoWpnname_t;

extern CsoWpnname_t g_szCsoWpnName[];

extern const wchar_t* GetWeaponLangName(const char* szName);
extern const char* CheckCSOName(CsoWpnname_t* csoname, const char* szName);
extern const char* GetEnhanceLevelName(int* iEnhanceData);
