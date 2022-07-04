#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif
#include "weapons/KnifeAttack.h"


namespace sv {
	LINK_ENTITY_TO_CLASS(pianogun_wave, CPianoGunWave)

	CPianoGunWave* CPianoGunWave::Create(int iWpnType, int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* Pent = CREATE_NAMED_ENTITY(MAKE_STRING("pianogun_wave"));

		if (FNullEnt(Pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CPianoGunWave* pWave = (CPianoGunWave*)Instance(Pent);

		if (pWave)
		{
			pWave->m_iWpnType = iWpnType;
			pWave->m_iType = iType;
			pWave->pev->owner = pentOwner;
			pWave->pev->origin = vecOrigin;
			pWave->pev->angles = vecAngles;
			pWave->m_iTeam = iTeam;
			pWave->Spawn();
		}

		return pWave;
	}

	void CPianoGunWave::Spawn(void)
	{
		Precache();


		pev->nextthink = gpGlobals->time + 0.1s;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255;

		pev->frame = 0;
		ResetSequenceInfo();
		switch (m_iType)
		{
		//PianoGun PrimaryAttack-> Notes 2
		case 0:
		{

			SET_MODEL(ENT(pev), "models/ef_pianogunwave_a.mdl");
			break;
		}
		//PianoGun SecondaryAttack-> Waves 2
		case 1:
		{
			m_flRemoveTime = gpGlobals->time + 0.6s;

			SET_MODEL(ENT(pev), "models/ef_pianogunwave_c.mdl");
			break;
		}
		case 2:
		{

			SET_MODEL(ENT(pev), "models/ef_pianogunwave_b.mdl");
			break;
		}
		case 3:
		{
			m_flRemoveTime = gpGlobals->time + 0.6s;

			SET_MODEL(ENT(pev), "models/ef_pianogunwave_c2.mdl");
			break;
		}
		//PianoGunEx PrimaryAttack-> Notes 3
		case 4: case 5: case 6:
		{
			break;
		}
		//PianoGunEx SecondaryAttack-> Waves 2
		case 7: case 8:
		{
			break;
		}
		default:break;

		}
		if (m_iType == 0 || m_iType == 1)
		{
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLY;

			SetTouch(&CPianoGunWave::FlyingTouch);
			SetThink(&CPianoGunWave::FlyingThink);

			UTIL_SetSize(pev, Vector(-6, -3, -3), Vector(6, 3, 3));
		}
		if (m_iType == 2 || m_iType == 3)
		{
			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_NONE;

			SetThink(&CPianoGunWave::AttachmentThink);

			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		}

	}

	void CPianoGunWave::Precache(void)
	{
		//WEAPON_PIANOGUN
		//7mdl + 13 spr + 10 sound in total
		PRECACHE_MODEL("models/ef_pianogunwave_a.mdl");
		PRECACHE_MODEL("models/ef_pianogunwave_b.mdl");
		PRECACHE_MODEL("models/ef_pianogunwave_c.mdl");
		PRECACHE_MODEL("models/ef_pianogunwave_c2.mdl");

		PRECACHE_SOUND("weapons/pianogun_exp1.wav");
		PRECACHE_SOUND("weapons/pianogun_exp2.wav");


		m_iPianoNoteExp[0] = PRECACHE_MODEL("sprites/ef_pianogun_hit_a.spr");
		m_iPianoNoteExp[1] = PRECACHE_MODEL("sprites/ef_pianogun_hit_c.spr");

		m_iPianoShootMuzzle = PRECACHE_MODEL("sprites/ef_piano_attack_c.spr");

	}

	int CPianoGunWave::GetWeaponsId(void) { return m_iWpnType ? WEAPON_PIANOGUN : WEAPON_PIANOGUN; }


	void CPianoGunWave::FlyingThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		/*if (m_iWpnType)
		{
		}*/

		if (!IsInWorld())
		{
			UTIL_Remove(this);
			return;
		}
	}
	void CPianoGunWave::AttachmentThink()
	{
		pev->nextthink = gpGlobals->time + 0.001s;

		//default frames
		this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime / 1s) + this->pev->frame;
		if (pev->frame > 18)
		{
			pev->frame = fmod(pev->frame, 18);
		}

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);



		if (!pAttackPlayer || pAttackPlayer->m_bIsZombie)
		{
			UTIL_Remove(this);
		}
		UTIL_MakeVectors(pAttackPlayer->pev->v_angle);

		Vector vecForward = gpGlobals->v_forward;
		vecForward.z = 0;
		vecForward = vecForward.Normalize();

		Vector vecSrc = pAttackPlayer->pev->origin;
		if (FBitSet(pAttackPlayer->pev->flags, FL_DUCKING))
			vecSrc.z -= 32.0;

		UTIL_SetOrigin(pev, vecSrc + vecForward * 50);

		pev->angles = pAttackPlayer->pev->angles;
		pev->angles.x = 0;

		if (m_flRemoveTime < gpGlobals->time)
		{
			//UTIL_Remove(this);
			SetThink(&CPianoGunWave::StartFadeOut);
			m_flRemoveTime = invalid_time_point;
			return;
		}
	}
	void CPianoGunWave::StartFadeOut()
	{
		if (pev->rendermode == kRenderNormal) {
			pev->renderamt = 255.0f;
			pev->rendermode = kRenderTransTexture;
		}

		pev->solid = SOLID_NOT;
		pev->avelocity = g_vecZero;
		pev->nextthink = gpGlobals->time + 0.1s;

		SetThink(&CPianoGunWave::FadeOut);
	}

	void CPianoGunWave::FadeOut()
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



	void CPianoGunWave::RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
	{
		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		float flAdjustedDamage, falloff;
		Vector vecSpot;

		if (flRadius)
		{
			falloff = flDamage / flRadius;

			if (g_pModRunning->DamageTrack() == DT_ZB)
				falloff *= 0.3;
		}
		else
			falloff = 1;

		int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);
		bool bHit = false;

		vecSrc.z += 1;

		if (!pevAttacker)
			pevAttacker = pevInflictor;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
				continue;

			if (bInWater && !pEntity->pev->waterlevel)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (!pEntity->IsPlayer())
				break;

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
			if (pEntity == pAttackPlayer)
				continue;

			if (pEntity->pev == pevAttacker)
				flDamage *= 0.3;

			vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1 || tr.pHit == pEntity->edict())
			{
				bool bDamageRate = false;
				float flDamageRate;

				if ((bitsDamageType & DMG_EXPLOSION) && false /* UTIL_IsGame("czero") */)
				{
					bDamageRate = true;
					flDamageRate = UTIL_CalculateDamageRate(vecSrc, pEntity);
				}
				else
				{
					flDamageRate = 1.0;
				}

				if (bDamageRate)
				{
					float dis = flRadius - (vecSrc - pEntity->pev->origin).Length();

					if (!flRadius)
						flRadius = flDamage;

					if (!flRadius)
						continue;

					flAdjustedDamage = (dis * dis) * 1.25 / (flRadius * flRadius) * flDamageRate * flDamage * 1.5;
				}
				else
				{
					flAdjustedDamage = (vecSrc - pEntity->pev->origin).Length() * falloff;
					flAdjustedDamage = flDamage - flAdjustedDamage;
				}

				if (flAdjustedDamage < 0.0)
					continue;

				pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);

				if (pEntity->pev->health > 0.0)
					continue;

				if (pEntity != CBaseEntity::Instance(pevAttacker))
					bHit = true;
			}
		}

	//	return bHit;
	}

	void CPianoGunWave::FlyingTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		float flDamage = GetDirectDamage();
		float flExplodeDamage = GetExplodeDamage();

		entvars_t* pevOwner = VARS(pev->owner);

		if (!pOther->IsDormant())
		{
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				TraceResult tr = UTIL_GetGlobalTrace();

				ClearMultiDamage();
				tr.iHitgroup = HITGROUP_CHEST;
				pOther->TraceAttack(pevOwner, flDamage, pev->velocity, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(pev, pevOwner);
			}
		}

		RadiusDamage(pev->origin, pev, pevOwner, flExplodeDamage, 3.0 * 39.37, CLASS_NONE, DMG_EXPLOSION | DMG_NEVERGIB);

		Vector vecDir = pev->velocity.Normalize();
		Vector vecOrigin = pev->origin;
		vecOrigin -= vecDir * 40;
		TraceResult tr;
		Vector vecEnd;

		vecEnd = vecOrigin;
		vecEnd.z -= 8192;

		UTIL_TraceLine(vecOrigin, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
		vecOrigin = tr.vecEndPos;


		if (!m_iWpnType)
		{
			switch (m_iType)
			{
			case 0:
			{
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + 20);
				WRITE_SHORT(m_iPianoNoteExp[0]);
				WRITE_BYTE(5);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();


				EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/pianogun_exp1.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
				break;
			}
			case 1:
			{
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + 20);
				WRITE_SHORT(m_iPianoNoteExp[1]);
				WRITE_BYTE(8);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();

				EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/pianogun_exp2.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

				if (pOther->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pOther;

					if (player->m_bIsZombie)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(2.0s, 1.0f);
							strategy->SetStunGravityTime(2.0s, 10.0f);
						}
					}
				}

				break;
			}
			}
		}


			SetTouch(NULL);
			SetThink(NULL);

			UTIL_Remove(this);
		}

		float CPianoGunWave::GetDirectDamage()
		{
			float flDamage;
			if (!m_iType)
			{
				flDamage = 35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 230;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 2500;
			}
			else if (m_iType == 1)
			{
				flDamage = 75;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 1500;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 3500;
			}
			return flDamage;
		}

		float CPianoGunWave::GetExplodeDamage()
		{
			float flDamage;
			if (!m_iType)
			{
				flDamage = 60;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 432;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 600;
			}
			else if (m_iType == 1)
			{
				flDamage = 100;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 700;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 1000;
			}
			return flDamage;
		}
	}
