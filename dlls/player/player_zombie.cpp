#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "client.h"

#include "gamemode/mods.h"
#include "gamemode/zb2/zb2_const.h"

void CBasePlayer::MakeZombie(ZombieLevel iEvolutionLevel)
{
	m_bIsZombie = true;
	m_bNotKilled = false;
	m_iZombieLevel = iEvolutionLevel;

	pev->body = 0;
	m_iModelName = iEvolutionLevel ? MODEL_ZOMBIE_ORIGIN : MODEL_ZOMBIE_HOST;

	const char *szModel = iEvolutionLevel ? "zombi_origin" : "zombi_host";
	SET_CLIENT_KEY_VALUE(entindex(), GET_INFO_BUFFER(edict()), "model", const_cast<char *>(szModel));

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	SetNewPlayerModel(szModelPath);


	UTIL_LogPrintf("\"%s<%i><%s><CT>\" triggered \"Became_ZOMBIE\"\n", STRING(pev->netname), GETPLAYERUSERID(edict()), GETPLAYERAUTHID(edict()));

	// remove guns & give nvg
	GiveDefaultItems();
	m_bNightVisionOn = false;
	ClientCommand("nightvision");

	// set default property
	pev->health = pev->max_health = 2000;
	pev->armortype = ARMOR_TYPE_HELMET;
	pev->armorvalue = 200;
	pev->gravity = 0.83f;
	ResetMaxSpeed();

}

void CBasePlayer::DeathSound_Zombie()
{
	// temporarily using pain sounds for death sounds
	switch (RANDOM_LONG(1, 2))
	{
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_death_1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_death_2.wav", VOL_NORM, ATTN_NORM); break;
	}
}

void CBasePlayer::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_hurt_01.wav", VOL_NORM, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_hurt_02.wav", VOL_NORM, ATTN_NORM); break;
	}
}

void PlayerZombie_Precache()
{
	PRECACHE_SOUND("zombi/zombi_death_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_2.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_02.wav");

	PRECACHE_SOUND("zombi/zombi_heal.wav");
}

float CBasePlayer::Zombie_AdjustDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	// grenade damage 5x in zb mode
	if (g_pModRunning->DamageTrack() == DT_ZB && !Q_strcmp(STRING(pevInflictor->classname), "grenade"))
	{
		if (bitsDamageType & DMG_EXPLOSION)
		{
			if (m_bIsZombie)
				flDamage *= 5.0f;
			else
				flDamage *= 2.5f;
		}
	}

	return flDamage = m_pModStrategy->AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}