#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"

const char *sPlayerModelFiles[] =
{
	"models/player.mdl",
	"models/player/leet/leet.mdl",
	"models/player/gign/gign.mdl",
	"models/player/vip/vip.mdl",
	"models/player/gsg9/gsg9.mdl",
	"models/player/guerilla/guerilla.mdl",
	"models/player/arctic/arctic.mdl",
	"models/player/sas/sas.mdl",
	"models/player/terror/terror.mdl",
	"models/player/urban/urban.mdl",
	"models/player/spetsnaz/spetsnaz.mdl",	// CZ
	"models/player/militia/militia.mdl"	// CZ
};

const char *Client_ApperanceToModel(int iApperance)
{
	return sPlayerModelFiles[iApperance];
}

void PlayerModel_Precache()
{
	for(auto psz : sPlayerModelFiles)
		PRECACHE_MODEL(const_cast<char *>(psz));
}

void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax)
{
	for (auto psz : sPlayerModelFiles)
		ENGINE_FORCE_UNMODIFIED(force_model_specifybounds, (float *)&vMin, (float *)&vMax, psz);
}
