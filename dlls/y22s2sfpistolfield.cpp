#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include <dlls/gamemode/mod_zb1.h>

#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(y22s2sfpistolfield, CY22s2SFpistolField)

	CY22s2SFpistolField* CY22s2SFpistolField::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("y22s2sfpistolfield"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CY22s2SFpistolField* pDragon = (CY22s2SFpistolField*)Instance(pent);

		if (pDragon)
		{
			pDragon->m_iType = iType;
			pDragon->pev->owner = pentOwner;

			pDragon->pev->origin.x = vecOrigin[0];
			pDragon->pev->origin.y = vecOrigin[1];
			pDragon->pev->origin.z = vecOrigin[2];

			//pDragon->pev->angles = vecAngles;
			pDragon->m_iTeam = iTeam;
			pDragon->Spawn();
			
		}

		return pDragon;
	}
	void CY22s2SFpistolField::Spawn(void)
	{
		Precache();

		pev->nextthink = gpGlobals->time + 0.1s;
		m_flAttackInterval = gpGlobals->time + 0.5s;
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;

		pev->frame = 0;
		pev->sequence = 0;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255;

		DROP_TO_FLOOR(ENT(pev));

		ResetSequenceInfo();

		m_flRemoveTime = gpGlobals->time + GetLifeTime();
		

		
		if (m_iType)
		{
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			SET_MODEL(ENT(pev), "models/ef_y22s2sfpistol.mdl");
			EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/y22s2sfpistol_shoot2_fx.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			SetThink(&CY22s2SFpistolField::OnThink);
		}
		else
		{
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			SET_MODEL(ENT(pev), "models/ef_y22s2sfpistol.mdl");
			EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/y22s2sfpistol_shoot2_fx.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			SetThink(&CY22s2SFpistolField::OnThink);
		}


		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	}
	void CY22s2SFpistolField::Precache(void)
	{
		//WEAPON_M3DRAGONM
		PRECACHE_MODEL("models/ef_y22s2sfpistol.mdl");	//ground
		PRECACHE_SOUND("weapons/y22s2sfpistol_shoot2_fx.wav");
	}
	int CY22s2SFpistolField::GetWeaponsId(void) { return m_iType ? WEAPON_Y22S2SFPISTOL : WEAPON_Y22S2SFPISTOL; }
	void CY22s2SFpistolField::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_flRemoveTime < gpGlobals->time)
		{
			m_flRemoveTime = invalid_time_point;
			pev->renderamt = 0.0f;
			pev->nextthink = gpGlobals->time + 0.2s;

			pev->solid = SOLID_NOT;
			pev->avelocity = g_vecZero;
			pev->nextthink = gpGlobals->time + 0.1s;
			SetThink(&CBaseEntity::SUB_Remove);
			return;
		}

		if (m_flAttackInterval != invalid_time_point && m_flAttackInterval < gpGlobals->time)
		{
			//Do KNOCKBACK
			DoDebuff();

			m_flAttackInterval = gpGlobals->time + 0.5s;
		}
	}
	void CY22s2SFpistolField::DoDebuff(void)
	{
		Vector vecSrc;
		Vector2D vecAngleCheckDir;
		CBaseEntity* pEntity = NULL;
		TraceResult tr;

		vecAngleCheckDir = gpGlobals->v_forward.Make2D().Normalize();
		vecSrc = pev->origin;

		entvars_t* pevOwner = VARS(pev->owner);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 200.0)) != NULL)
		{
			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(3.0s,150.0f);
						strategy->SetStunGravityTime(3.0s, 150.0f);
					}
				}
			}
		}
	}

	float CY22s2SFpistolField::GetAttackRange()
	{
		return m_iType ? 140.0 : 160.0;
	}

	duration_t CY22s2SFpistolField::GetLifeTime()
	{
		return 3.0s;
	}


}