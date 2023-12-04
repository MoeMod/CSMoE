#include <stdio.h>
#include "Util.h"

#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <VGUI/ILocalize.h>

using namespace vgui2;

const char* szLangKeyList[3] = { "#CSO_Item_Name_%s", "#CSO_%s", "#CSBTE_%s" };

//Fix Names? From Sme...
CsoWpnname_t g_szCsoWpnName[] =
{
	{ "usp", "USP45" },
	{ "deagle", "DesertEagle" },
	{ "ak47_long", "AK47L" },
	{ "dmp7a1", "MP7A1D" },
	{ "ddeagle", "DeagleD" },
	{ "r93", "blaser93" },
	{ "flash", "Flashbang" },
	{ "vest", "Kevlar" },
	{ "vesthelm", "Kevlar_Helmet" },
	{ "sgren", "SmokeGrenade" },
	{ "nvgs", "nightvision" },
	{ "elite", "Elites" },
	{ "knife", "defaultknife" },
	{ "g_ak47", "ak47g" },
	{ "g_m4a1", "m4a1g" },
	{ "usas", "usas12" },
	{ "g_deagle", "DesertEagleG" },
	{ "cake", "CakeBomb" },
	{ "fgrenade", "FireBomb" },
	{ "automagv", "automag" },
	{ "strong_knife", "NataKnife" },
	{ "laserminigunpaint01", "laserminigunpaintlaserminigunonly" },
	{ NULL, NULL },
};


const wchar_t* GetWeaponLangName(const char* szName)
{
	if (!szName)
		return L"";

	//Firstly,Localize?
	char szKey[64];
	for (int i = 0; i < 3; i++)
	{
		Q_snprintf(szKey, 64, szLangKeyList[i], szName);
		if (wchar_t* p = localize()->Find(szKey))
			return p;
	}

	//Else
	wchar_t szKey_W[64];
	Q_UTF8ToUnicode(szKey, szKey_W, 63);

	return szKey_W;
}

const char* CheckCSOName(CsoWpnname_t* csoname, const char* szName)
{
	if (!szName)
		return nullptr;
	if (!csoname)
		return szName;

	for (int i = 0; csoname[i].source != NULL; i++)
	{
		if (!stricmp(szName, csoname[i].source))
			return csoname[i].dest;
	}

	return szName;
}

const char* GetEnhanceLevelName(int* iEnhanceData)
{
	int damage, accuracy, rebound, weight, repeatedly, load;

	damage = iEnhanceData[0];
	accuracy = iEnhanceData[1];
	rebound = iEnhanceData[2];
	weight = iEnhanceData[3];
	repeatedly = iEnhanceData[4];
	load = iEnhanceData[5];


	if (damage >= 5)
		return "#CSO_WpnTitle_D5";

	if (accuracy >= 5)
		return "#CSO_WpnTitle_A5";

	if (rebound >= 5)
		return "#CSO_WpnTitle_K5";

	if (weight >= 5)
		return "#CSO_WpnTitle_W5";

	if (repeatedly >= 5)
		return "#CSO_WpnTitle_S5";

	if (load >= 5)
		return "#CSO_WpnTitle_B5";

	if (damage >= 3 && accuracy >= 3)
		return "#CSO_WpnTitle_D3A3";

	if (damage >= 3 && rebound >= 3)
		return "#CSO_WpnTitle_D3K3";

	if (damage >= 3 && weight >= 3)
		return "#CSO_WpnTitle_D3W3";

	if (damage >= 3 && repeatedly >= 3)
		return "#CSO_WpnTitle_D3S3";

	if (damage >= 3 && load >= 3)
		return "#CSO_WpnTitle_D3B3";

	if (accuracy >= 3 && rebound >= 3)
		return "#CSO_WpnTitle_A3K3";

	if (accuracy >= 3 && weight >= 3)
		return "#CSO_WpnTitle_A3W3";

	if (accuracy >= 3 && repeatedly >= 3)
		return "#CSO_WpnTitle_A3S3";

	if (accuracy >= 3 && load >= 3)
		return "#CSO_WpnTitle_A3B3";

	if (rebound >= 3 && weight >= 3)
		return "#CSO_WpnTitle_K3W3";

	if (rebound >= 3 && repeatedly >= 3)
		return "#CSO_WpnTitle_K3S3";

	if (rebound >= 3 && load >= 3)
		return "#CSO_WpnTitle_K3B3";

	if (weight >= 3 && repeatedly >= 3)
		return "#CSO_WpnTitle_W3S3";

	if (weight >= 3 && load >= 3)
		return "#CSO_WpnTitle_W3B3";

	if (repeatedly >= 3 && load >= 3)
		return "#CSO_WpnTitle_S3B3";

	return NULL;
}