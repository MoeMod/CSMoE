#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
	LINK_ENTITY_TO_CLASS(m3dragoncannon_dragon, CM3DragonCannon_Dragon)

	CM3DragonCannon_Dragon* CM3DragonCannon_Dragon::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("m3dragoncannon_dragon"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CM3DragonCannon_Dragon* pDragon = (CM3DragonCannon_Dragon*)Instance(pent);

		if (pDragon)
		{
			pDragon->m_iType = iType;
			pDragon->pev->owner = pentOwner;
			pDragon->pev->origin = vecOrigin;
			pDragon->pev->angles = vecAngles;
			pDragon->m_iTeam = iTeam;
			pDragon->Spawn();
		}

		return pDragon;
	}

	void CM3DragonCannon_Dragon::Spawn(void)
	{
		Precache();
	
		pev->nextthink = gpGlobals->time + 0.1s;
		m_flAttackInterval = gpGlobals->time + 1.0s;
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;

		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();

		m_flRemoveTime = gpGlobals->time + GetLifeTime();
		if (m_iType)
		{
			m_flCreateSharkTime = gpGlobals->time + 1.0s;
			SET_MODEL(ENT(pev), "models/ef_m3dragonm_sign.mdl");
			EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/m3dragonm_shark_fx.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			SetThink(&CM3DragonCannon_Dragon::OnSharkThink);
		}
			
		else
		{
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			SET_MODEL(ENT(pev), "models/m3dragon_effect.mdl");
			EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/m3dragon_dragon_fx.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			SetThink(&CM3DragonCannon_Dragon::OnThink);
		}
			

		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	}

	void CM3DragonCannon_Dragon::Precache(void)
	{
		//WEAPON_M3DRAGONM
		PRECACHE_MODEL("models/ef_m3dragonm_sign.mdl");	//ground
		m_iSharkEffect = PRECACHE_MODEL("models/m3dragonm_effect.mdl");	//shark
		m_iWaterEffect = PRECACHE_MODEL("sprites/ef_m3dragonm_water.spr");	//splash
		PRECACHE_SOUND("weapons/m3dragonm_shark_fx.wav");

		//WEAPON_M3DRAGON
		PRECACHE_MODEL("models/m3dragon_effect.mdl");
		PRECACHE_SOUND("weapons/m3dragon_dragon_fx.wav");
	}

	int CM3DragonCannon_Dragon::GetWeaponsId(void) { return m_iType ? WEAPON_M3DRAGONM : WEAPON_M3DRAGON; }

	bool CM3DragonCannon_Dragon::CylinderDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, float flHeight, float flKnockBack, Vector vecDir, int bitsDamageType, bool bDistanceCheck)
	{
		float falloff;
		bool bHit = false;

		if (bDistanceCheck)
			falloff = flDamage / flRadius;
		else
			falloff = 0.0;

		if (!pevAttacker)
			pevAttacker = pevInflictor;

		float fCheckRange = flHeight > flRadius ? flHeight : flRadius;
		float flAdjustedDamage;

		Vector vecEnd, vecOrigin, vecVelocity;
		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, fCheckRange)) != NULL)
		{
			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			vecEnd = pEntity->BodyTarget(pev->origin);
			vecOrigin = vecEnd - vecSrc;
			vecOrigin[2]= vecEnd[2];
			if (vecOrigin.Length2D() > flRadius)
				continue; // Far than circle range //(x-a)^2 + (y-b)^2 > r^2 :V
			if (vecOrigin[2] > vecSrc[2] + flHeight || vecOrigin[2] < vecSrc[2])
				continue; // Far than cylinder height

			float flDistance = vecOrigin.Length();
			if (flDistance < 1.0)
				flDistance = 0.0;

			flAdjustedDamage = flDistance * falloff;
			flAdjustedDamage = flDamage - flAdjustedDamage;

			pEntity->TakeDamage(pev, pevAttacker, flAdjustedDamage, bitsDamageType);	

			bHit = true;

			if(g_pGameRules->IsTeamplay() && pEntity->m_iTeam == m_iTeam)
				continue;

			if (vecDir.Length())
			{
				Vector vecAngle = vecDir.Normalize();
				if (bDistanceCheck)
					flKnockBack *= (flRadius - flDistance) / flRadius;
				vecVelocity = vecAngle * flKnockBack;

			}
			else
				vecVelocity = vecEnd.Normalize() * flKnockBack * ((flRadius - flDistance) / flRadius);
			
			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;
				player->pev->velocity += vecVelocity;
			}
			else
				pEntity->pev->velocity += vecVelocity;
			
		}

		return bHit;
	}

	void CM3DragonCannon_Dragon::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_flRemoveTime < gpGlobals->time)
		{
			SetThink(&CM3DragonCannon_Dragon::StartFadeOut);
			m_flRemoveTime = invalid_time_point;
			return;
		}

		if (m_flAttackInterval != invalid_time_point && m_flAttackInterval < gpGlobals->time)
		{
			Vector vecOrigin = pev->origin;
			vecOrigin[2] -= 15;
			float flHeight = 500;

			flHeight *= (1.0 - (m_flRemoveTime - gpGlobals->time) / GetLifeTime());

			entvars_t* pevOwner = VARS(pev->owner);

			CylinderDamage(vecOrigin, this->pev, pevOwner, GetDamage(), GetAttackRange(), flHeight, 220, Vector {0.0, 0.0, 1.0}, DMG_BULLET | DMG_NOMORALE | DMG_NEVERGIB, FALSE);

			m_flAttackInterval = gpGlobals->time + 0.2s;
		}
	}


	void CM3DragonCannon_Dragon::StartFadeOut()
	{
		if (pev->rendermode == kRenderNormal) {
			pev->renderamt = 255.0f;
			pev->rendermode = kRenderTransTexture;
		}

		pev->solid = SOLID_NOT;
		pev->avelocity = g_vecZero;
		pev->nextthink = gpGlobals->time + 0.1s;

		SetThink(&CM3DragonCannon_Dragon::FadeOut);
	}

	void CM3DragonCannon_Dragon::FadeOut()
	{
		if (pev->renderamt > 15) {
			pev->renderamt -= 35.0f;
			pev->nextthink = gpGlobals->time + 0.1s;
		}
		else {
			pev->renderamt = 0.0f;
			pev->nextthink = gpGlobals->time + 0.2s;
			SetThink(&CBaseEntity::SUB_Remove);
		}
	}

	void CM3DragonCannon_Dragon::OnSharkThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_flRemoveTime < gpGlobals->time)
		{
			SetThink(&CM3DragonCannon_Dragon::StartFadeOut);
			m_flRemoveTime = invalid_time_point;
			return;
		}

		if (m_flCreateSharkTime < gpGlobals->time)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_TEMPMODEL);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(m_iSharkEffect);
			WRITE_BYTE(20);	//life
			WRITE_SHORT(0);
			WRITE_BYTE(10);
			WRITE_BYTE(false);
			WRITE_BYTE(255);
			WRITE_BYTE(kRenderNormal);
			WRITE_SHORT(-1);
			WRITE_BYTE(0);
			WRITE_BYTE(false);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_SHORT(61);
			WRITE_LONG(0);
			MESSAGE_END();


			for (int i = 0; i < 4; i++)
			{
				Vector vecOrigin = pev->origin;
				if (i % 2)
				{
					vecOrigin.x += (i == 1 ? -1.0 : 1.0) * 50.0;
					vecOrigin.y += (i == 1 ? 1.0 : -1.0) * 50.0;
				}
				else
				{
					vecOrigin.x += (i == 1 ? -1.0 : 1.0) * 50.0;
					vecOrigin.y += (i == 1 ? -1.0 : 1.0) * 50.0;
				}
				vecOrigin.z += 25.0;

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(vecOrigin.x);
				WRITE_COORD(vecOrigin.y);
				WRITE_COORD(vecOrigin.z);
				WRITE_SHORT(m_iWaterEffect);
				WRITE_BYTE(8);
				WRITE_BYTE(255);
				WRITE_BYTE(15);
				MESSAGE_END();
			}


			m_flCreateSharkTime = gpGlobals->time + 9999s;
		}

		if (m_flAttackInterval != invalid_time_point && m_flAttackInterval < gpGlobals->time)
		{
			Vector vecOrigin = pev->origin;
			vecOrigin[2] -= 15;
			float flHeight = 500;

			flHeight *= (1.0 - (m_flRemoveTime - gpGlobals->time) / GetLifeTime());

			entvars_t* pevOwner = VARS(pev->owner);
			
			CylinderDamage(vecOrigin, this->pev, pevOwner, GetDamage(), GetAttackRange(), flHeight, 220, Vector{ 0.0, 0.0, 1.0 }, DMG_BULLET | DMG_NOMORALE | DMG_NEVERGIB, FALSE);

			m_flAttackInterval = gpGlobals->time + 0.2s;
		}
	}

	float CM3DragonCannon_Dragon::GetDamage()
	{		
		float flDamage;
		if (m_iType)
		{
			flDamage = 4;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 1350;
				else
					flDamage = 650;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1820;
		}
		else
		{
			flDamage = 6;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 2100;
				else
					flDamage = 1350;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 2800;
		}
		return flDamage;
	}

	float CM3DragonCannon_Dragon::GetAttackRange()
	{
		return m_iType ? 140.0 : 160.0;
	}

	duration_t CM3DragonCannon_Dragon::GetLifeTime()
	{
		return 3.0s;
	}
}
