#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_hegrenade.h"
#include "soundent.h"
#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CStickyGrenade : public CGrenade
	{
	public:
		void Spawn()
		{
			CGrenade::Spawn();

			Precache();
			m_iGrenadeID = GRENADE_STICKYBOMB;

			SET_MODEL(ENT(pev), "models/w_stickybomb.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			m_flSpawnTime = gpGlobals->time;
		}

		void Precache()
		{
			PRECACHE_MODEL("models/w_stickybomb.mdl");

			m_iModelIndexExplosionW = PRECACHE_MODEL("sprites/ef_stickybomb_explosion_w.spr");
			m_iModelIndexExplosionR = PRECACHE_MODEL("sprites/ef_stickybomb_explosion_r.spr");
			m_iModelIndexExplosionG = PRECACHE_MODEL("sprites/ef_stickybomb_explosion_g.spr");

			m_iModelIndexSmokeW = PRECACHE_MODEL("sprites/ef_stickybomb_smoke_w.spr");
			m_iModelIndexSmokeR = PRECACHE_MODEL("sprites/ef_stickybomb_smoke_r.spr");
			m_iModelIndexSmokeG = PRECACHE_MODEL("sprites/ef_stickybomb_smoke_g.spr");

			PRECACHE_MODEL("sprites/firebomb_burn.spr");

			PRECACHE_SOUND("weapons/stickybomb_red_exp.wav");
			PRECACHE_SOUND("weapons/stickybomb_green_exp.wav");
			PRECACHE_SOUND("weapons/stickybomb_white_exp.wav");

			m_usEvent = PRECACHE_EVENT(1, "events/createexplo.sc");
		}

	public:
		void EXPORT StickGrenadeTouch(CBaseEntity* pOther)
		{
			SetTouch(NULL);
			SetThink(&CStickyGrenade::SensorThink);

			pev->frame = 0;
			pev->sequence = 0;

			ResetSequenceInfo();

			pev->nextthink = gpGlobals->time;

			if (pOther->IsBSPModel())
			{
				pev->movetype = MOVETYPE_NONE;

				return;
			}

			if (!pOther->IsAlive())
				return;

			if (pOther->pev->takedamage == DAMAGE_NO)
				return;

			pev->effects |= EF_NODRAW;
			pev->velocity = g_vecZero;
			pev->solid = SOLID_NOT;

			m_pStickingObject = pOther;

			MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
			WRITE_BYTE(TE_FOLLOWINGGRENADE);
			WRITE_SHORT(pOther->entindex());
			WRITE_SHORT(pev->modelindex);
			WRITE_COORD(pev->origin.x - pOther->pev->origin.x);
			WRITE_COORD(pev->origin.y - pOther->pev->origin.y);
			WRITE_COORD(pev->origin.z - pOther->pev->origin.z);
			WRITE_COORD(0.0f);
			WRITE_COORD(1.0f);
			WRITE_COORD(0.0f);
			WRITE_SHORT(300.0f);
			WRITE_BYTE(pev->body);
			WRITE_BYTE(pev->skin);
			MESSAGE_END();
		}

		void EXPORT StickGrenadeExploThink()
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			if (!IsInWorld())
			{
				SetThink(nullptr);
				SetTouch(nullptr);
				UTIL_Remove(this);

				return;
			}
		}

		void EXPORT SensorThink()
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			if (!m_pOwner)
			{
				SetThink(nullptr);
				SetTouch(nullptr);
				UTIL_Remove(this);
				return;
			}

			if (m_pOwner->m_bIsZombie)
			{
				SetThink(nullptr);
				SetTouch(nullptr);
				UTIL_Remove(this);
				return;
			}


			Vector vecSrc = pev->origin;

			if (m_pStickingObject.Get() != NULL)
			{
				CBaseEntity* pEntity = m_pStickingObject;

				vecSrc = pEntity->pev->origin;

				if (!pEntity->IsAlive())
				{
					MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
					WRITE_SHORT(pEntity->entindex());
					MESSAGE_END();

					pev->effects &= ~EF_NODRAW;
					pev->solid = SOLID_BBOX;

					SetTouch(&CStickyGrenade::StickGrenadeTouch);
				}		
			}

			CBaseEntity* pEntity = NULL;
			TraceResult tr;
			
			float flRadius = 39.37 * 3.5;

			bool bExplode = false;
			bool bInWater = UTIL_PointContents(vecSrc) == CONTENTS_WATER;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
			{
				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity == m_pStickingObject)
					continue;

				if (pEntity == m_pOwner)
					continue;

				if (bInWater && pEntity->pev->waterlevel == 0)
					continue;

				if (!bInWater && pEntity->pev->waterlevel == 3)
					continue;

				if (pEntity->Classify() == CLASS_PLAYER || pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					UTIL_TraceLine(vecSrc, pEntity->BodyTarget(vecSrc), dont_ignore_monsters, pev->owner, &tr);

					if (tr.flFraction < 1.0f && tr.pHit != pEntity->edict())
						continue;

					if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
						continue;

					bExplode = true;
				}
			}

			if (bExplode)
				StickGrenadeDetonate();

		}

		void EXPORT Smoke()
		{
			if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
			{
				UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
			}
			else
			{
				if (m_iType == 1)
				{
					MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pev->origin.x);
					WRITE_COORD(pev->origin.y);
					WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
					WRITE_SHORT(m_iModelIndexSmokeG);
					WRITE_BYTE(35 + RANDOM_FLOAT(0, 10)); // scale * 10
					WRITE_BYTE(255);
					WRITE_BYTE(30);
					MESSAGE_END();
				}
				else if (m_iType == 2)
				{
					MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pev->origin.x);
					WRITE_COORD(pev->origin.y);
					WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
					WRITE_SHORT(m_iModelIndexSmokeW);
					WRITE_BYTE(35 + RANDOM_FLOAT(0, 10)); // scale * 10
					WRITE_BYTE(255);
					WRITE_BYTE(30);
					MESSAGE_END();
				}
				else
				{
					MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pev->origin.x);
					WRITE_COORD(pev->origin.y);
					WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
					WRITE_SHORT(m_iModelIndexSmokeR);
					WRITE_BYTE(35 + RANDOM_FLOAT(0, 10)); // scale * 10
					WRITE_BYTE(255);
					WRITE_BYTE(30);
					MESSAGE_END();
				}			
			}

			UTIL_Remove(this);
		}

		void EXPORT StickGrenadeDetonate()
		{
			SetThink(nullptr);

			TraceResult tr;

			if (m_pStickingObject.Get() != NULL)
			{
				CBaseEntity* pEntity = m_pStickingObject;

				if (pEntity)
				{
					pev->origin = pEntity->pev->origin;
				}


				MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
				WRITE_SHORT(pEntity->entindex());
				MESSAGE_END();
			}

			UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 40), ignore_monsters, edict(), &tr);

			pev->model = iStringNull; // invisible
			pev->solid = SOLID_NOT; // intangible
			pev->takedamage = DAMAGE_NO;

			if (tr.flFraction != 1.0f)
			{
				pev->origin = tr.vecEndPos + (tr.vecPlaneNormal * (pev->dmg - 24.0f) * 0.6f);
			}
	
			if (m_iType == 1)
			{
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
				WRITE_SHORT(m_iModelIndexExplosionG);
				WRITE_BYTE(30);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stickybomb_green_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
				
			else if (m_iType == 2)
			{
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
				WRITE_SHORT(m_iModelIndexExplosionW);
				WRITE_BYTE(30);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stickybomb_white_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}			
			else
			{
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
				WRITE_SHORT(m_iModelIndexExplosionR);
				WRITE_BYTE(30);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stickybomb_red_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}

			CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3s);


			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_HE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
			}

			pev->owner = NULL;

			StickGrenadeExplode(pev->origin, pev, m_pevOwner, m_flExplodeDamage, 0.1, 325);

			pev->effects |= EF_NODRAW;
			SetThink(&CStickyGrenade::Smoke);
			pev->velocity = g_vecZero;
			pev->nextthink = gpGlobals->time + 0.1s;

			CBasePlayer* player = GetClassPtr<CBasePlayer>(m_pevOwner);
			if (player)
			{
				player->m_pModStrategy->GrenadeExploded(pev->origin);
			}
		}

	public:
		void StickGrenadeExplode(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flBurnAmount, float flRadius)
		{
			CBaseEntity* pEntity = NULL;
			TraceResult tr;
			Vector vecEnd;

			float falloff;

			if (flRadius)
				falloff = flDamage / flRadius;
			else
				falloff = 1.0f;

			bool bInWater = UTIL_PointContents(vecSrc) == CONTENTS_WATER;

			vecSrc.z += 1.0;

			entvars_t* pevAttack = pevAttacker ? pevAttacker : pevInflictor;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
			{
				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (bInWater && pEntity->pev->waterlevel == 0)
					continue;

				if (!bInWater && pEntity->pev->waterlevel == 3)
					continue;

				if (pEntity == m_pOwner)
					continue;

				if (pEntity->Classify() == CLASS_PLAYER || pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					UTIL_TraceLine(vecSrc, pEntity->BodyTarget(vecSrc), dont_ignore_monsters, ENT(pevInflictor), &tr);

					if (tr.flFraction < 1.0f && tr.pHit != pEntity->edict())
						continue;

					if (tr.fStartSolid)
					{
						tr.vecEndPos = vecSrc;
						tr.flFraction = 0;
					}

					float flCurrentDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;

					pEntity->TakeDamage(pevInflictor, pevAttack, flCurrentDamage, DMG_EXPLOSION);

					if (m_iType == 1)
					{
						if (pEntity->IsPlayer())
						{
							CBasePlayer* player = (CBasePlayer*)pEntity;

							if (player->m_bIsZombie)
							{
								auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
								if (strategy)
								{
									strategy->SetStunSpeedTime(5.0s, 70.0f);
									strategy->SetStunGravityTime(5.0s);
								}
							}
						}
					}
					else if (m_iType == 2)
					{
						Vector vecDirection = (pEntity->pev->origin - m_pOwner->pev->origin).Normalize();
						UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 3500.0, 200);
					}
					else
					{
						if (pEntity->Classify() == CLASS_PLAYER_ALLY)
						{
							MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
							WRITE_BYTE(4);
							WRITE_SHORT(pEntity->entindex());
							WRITE_BYTE(18);
							MESSAGE_END();
						}
						else
						{
							if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) != GR_TEAMMATE)
							{
								MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
								WRITE_BYTE(4);
								WRITE_SHORT(pEntity->entindex());
								WRITE_BYTE(18);
								MESSAGE_END();

								pEntity->m_pBuffAttacker = pevAttacker;
								pEntity->m_iHealthDecreaseCount = 5;
								pEntity->m_flHealthDecreaseAmount = pEntity->pev->max_health * flBurnAmount;
								pEntity->m_flHealthDecreaseInterval = 1.0s;
								pEntity->m_flHealthDecreaseStartTime = gpGlobals->time + 1.0s;
							}
						}
					}
					

				}
				else if (pEntity->Classify() != CLASS_PLAYER_ALLY)
					continue;

				// ..
			}
		}

	public:
		void Initialize(const Vector& vecVelocity, int iTeam, int iType, CBasePlayer* pOwner)
		{
			m_iType = iType;
			m_bStaticFramerate = true;
			pev->velocity = vecVelocity;
			m_pStickingObject = NULL;

			SetTouch(&CStickyGrenade::StickGrenadeTouch);
			SetThink(&CStickyGrenade::StickGrenadeExploThink);

			pev->nextthink = gpGlobals->time + 0.1s;

			pev->frame = 0;
			pev->sequence = 1;

			ResetSequenceInfo();

			if (m_iType == 1)
			{
				pev->skin = 2;
				pev->body = 2;
			}		
			else if (m_iType == 2)
			{
				pev->skin = 1;
				pev->body = 1;
			}

			m_bJustBlew = true;

			pev->gravity = 0.55;
			pev->friction = 0.7;

			m_iTeam = iTeam;

			if (pev->owner)
				m_pevOwner = VARS(pev->owner);
			else
				m_pevOwner = NULL;

			m_pOwner = pOwner;
			pev->dmg = 100;
			m_flExplodeDamage = GetExplodeDamage();

			std::vector < std::pair<int, time_point_t >> vecEnt;
			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityByClassname(pEntity, "grenade")) != NULL)
			{
				CGrenade* grenade = dynamic_ent_cast<CGrenade*>(pEntity);
				if (grenade == nullptr)
					continue;

				if (grenade->m_iGrenadeID != GRENADE_STICKYBOMB)
					continue;

				CStickyGrenade* pGrenade = dynamic_ent_cast<CStickyGrenade*>(grenade);
				if (pGrenade == nullptr)
					continue;

				if (pGrenade->m_pOwner != this->m_pOwner)
					continue;

				vecEnt.emplace_back(pGrenade->entindex(), pGrenade->m_flSpawnTime);
			}
			
			if (!vecEnt.empty() && vecEnt.size() > 4)
			{
				std::sort(vecEnt.begin(), vecEnt.end(), [](const std::pair<int, time_point_t>& a, const std::pair<int, time_point_t>& b) {return a.second < b.second; });

				CStickyGrenade* pGrenade = (CStickyGrenade*)Instance(INDEXENT(vecEnt.at(0).first));
				if (pGrenade)
				{
					pGrenade->pev->effects |= EF_NODRAW;
					pGrenade->pev->flags |= FL_KILLME;
					pGrenade->SetThink(&CBaseEntity::SUB_Remove);
					pGrenade->SetTouch(NULL);
				}			
			}

		}

	public:

		static void ShootTimed(const Vector& vecOrigin, const Vector& vecAngles, const Vector& vecVelocity, int iTeam, int iType, CBasePlayer* pOwner)
		{
			CStickyGrenade* pGrenade = (CStickyGrenade*)CBaseEntity::Create("stickygrenade", vecOrigin, vecAngles, pOwner->edict());

			if (!pGrenade)
				return;

			SET_MODEL(ENT(pGrenade->pev), "models/w_stickybomb.mdl");
			pGrenade->Initialize(vecVelocity, iTeam, iType, pOwner);
		}

		float GetExplodeDamage()
		{
			float flDamage;

			switch (m_iType)
			{
			case 1:
			{
				flDamage = 120.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 4000.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 5200.0f;
				break;
			}
			case 2:
			{
				flDamage = 110.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 4000.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 5200.0f;
				break;
			}
			default:
			{
				flDamage = 100.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 4000.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 5200.0f;
				break;
			}
			
			}
			return flDamage;
		}


	public:
		int m_iModelIndexExplosionW;
		int m_iModelIndexExplosionR;
		int m_iModelIndexExplosionG;
		int m_iModelIndexSmokeW;
		int m_iModelIndexSmokeR;
		int m_iModelIndexSmokeG;
		int m_iType;
		float m_flExplodeDamage;
		time_point_t m_flSpawnTime;
		entvars_t* m_pevOwner;
		CBasePlayer* m_pOwner;
		EHANDLE m_pStickingObject;
	};

	LINK_ENTITY_TO_CLASS(stickygrenade, CStickyGrenade);
#endif


enum stickybomb_e
{
	STICKYBOMB_RED_IDLE,
	STICKYBOMB_RED_PULLPIN,
	STICKYBOMB_RED_THROW,
	STICKYBOMB_RED_DRAW,
	STICKYBOMB_RED_TRANSFORM1,
	STICKYBOMB_RED_TRANSFORM2,
	STICKYBOMB_WHITE_IDLE,
	STICKYBOMB_WHITE_PULLPIN,
	STICKYBOMB_WHITE_THROW,
	STICKYBOMB_WHITE_DRAW,
	STICKYBOMB_WHITE_TRANSFORM1,
	STICKYBOMB_WHITE_TRANSFORM2,
	STICKYBOMB_GREEN_IDLE,
	STICKYBOMB_GREEN_PULLPIN,
	STICKYBOMB_GREEN_THROW,
	STICKYBOMB_GREEN_DRAW,
	STICKYBOMB_GREEN_TRANSFORM1,
	STICKYBOMB_GREEN_TRANSFORM2,
};

LINK_ENTITY_TO_CLASS(weapon_stickybomb, CStickyBomb)

void CStickyBomb::Spawn(void)
{
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_stickybomb.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CStickyBomb::Precache(void)
{
	PRECACHE_MODEL("models/v_stickybomb.mdl");
	PRECACHE_MODEL("models/p_stickybomb.mdl");
	PRECACHE_MODEL("models/w_stickybomb.mdl");

	PRECACHE_MODEL("sprites/ef_stickybomb_transform_w.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_transform_r.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_transform_g.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_smoke_w.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_smoke_r.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_smoke_g.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_idle_r.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_idle_g.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_hand_w.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_hand_w2.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_hand_r.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_hand_r2.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_hand_g.spr");
	PRECACHE_MODEL("sprites/ef_stickybomb_hand_g2.spr");

	PRECACHE_SOUND("weapons/stickybomb_red_idle.wav");
	PRECACHE_SOUND("weapons/stickybomb_green_idle.wav");
	PRECACHE_SOUND("weapons/stickybomb_white_idle.wav");

	m_usFire = PRECACHE_EVENT(1, "events/stickybomb.sc");
}

int CStickyBomb::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "HEGrenade";
	p->iMaxAmmo1 = MAX_AMMO_HEGRENADE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_HEGRENADE;
	p->iWeight = HEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CStickyBomb::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;
	m_iLastAnim = 0;

	int iAnim;

	switch (m_iWeaponState)
	{
	case 1:
		iAnim = STICKYBOMB_GREEN_DRAW;
		pev->skin = 2;
		break;
	case 2: 
		iAnim = STICKYBOMB_WHITE_DRAW; 
		pev->skin = 1;
		break;
	default:
		iAnim = STICKYBOMB_RED_DRAW;
		pev->skin = 0;
		break;
	}

	if (DefaultDeploy("models/v_stickybomb.mdl", "models/p_stickybomb.mdl", iAnim, "grenade", UseDecrement() != FALSE))
	{
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_iWeaponState, 1, TRUE, TRUE);

		return TRUE;
	}

	return FALSE;
}

BOOL CStickyBomb::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CStickyBomb::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->RemoveWeapon(m_iId);
		DestroyItem();
	}

	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
}

void CStickyBomb::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;

		int iAnim;
		switch (m_iWeaponState)
		{
		case 1: iAnim = STICKYBOMB_GREEN_PULLPIN; break;
		case 2: iAnim = STICKYBOMB_WHITE_PULLPIN; break;
		default: iAnim = STICKYBOMB_RED_PULLPIN; break;
		}

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}


void CStickyBomb::SecondaryAttack(void)
{
	int iAnim;
	switch (m_iWeaponState)
	{
	case 0: 
		iAnim = STICKYBOMB_RED_TRANSFORM1; 
		m_iWeaponState = 1;
		break;
	case 1: 
		iAnim = STICKYBOMB_GREEN_TRANSFORM1; 
		m_iWeaponState = 2;
		break;
	default: 
		iAnim = iAnim = STICKYBOMB_WHITE_TRANSFORM1;
		m_iWeaponState = 0;
		break;
	}

	SendWeaponAnim(iAnim, UseDecrement() != FALSE);

	m_iLastAnim = iAnim;

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.23s;
}

void CStickyBomb::ItemPostFrame()
{
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase() && m_flNextSecondaryAttack > 0s)
	{
		if (m_iLastAnim != 0)
		{
			int iAnim;

			switch (m_iLastAnim)
			{
			case STICKYBOMB_RED_TRANSFORM1:
				iAnim = STICKYBOMB_GREEN_TRANSFORM2;
				pev->skin = 2;
				break;
			case STICKYBOMB_GREEN_TRANSFORM1:
				iAnim = STICKYBOMB_WHITE_TRANSFORM2;
				pev->skin = 1;
				break;
			default:
				iAnim = STICKYBOMB_RED_TRANSFORM2;
				pev->skin = 0;
				break;
			}

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_iWeaponState, 1, TRUE, TRUE);

			SendWeaponAnim(iAnim, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.47s;
			m_iLastAnim = 0;
			return;
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CStickyBomb::WeaponIdle(void)
{
	if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow != invalid_time_point)
	{	
#ifndef CLIENT_DLL
		m_pPlayer->Radio("%!MRAD_FIREINHOLE", "#Fire_in_the_hole");
#endif
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;

		auto time = 1.5s;
#ifndef CLIENT_DLL
		CStickyGrenade::ShootTimed(vecSrc, m_pPlayer->pev->angles, vecThrow, m_pPlayer->m_iTeam, m_iWeaponState, m_pPlayer);

		m_pPlayer->m_pModStrategy->ShootGrenade(std::bind(CStickyGrenade::ShootTimed, vecSrc, m_pPlayer->pev->angles, vecThrow, m_pPlayer->m_iTeam, m_iWeaponState, m_pPlayer));
#endif

		int iAnim;
		switch (m_iWeaponState)
		{
		case 1:
			iAnim = STICKYBOMB_GREEN_THROW;
			break;
		case 2:
			iAnim = STICKYBOMB_WHITE_THROW;
			break;
		default:
			iAnim = STICKYBOMB_RED_THROW;
			break;
		}


		SendWeaponAnim(iAnim, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_flStartThrow = invalid_time_point;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;

		return;
	}
	else if (m_flReleaseThrow != invalid_time_point)
	{
		m_flStartThrow = invalid_time_point;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			int iAnim;

			switch (m_iWeaponState)
			{
			case 1:
				iAnim = STICKYBOMB_GREEN_DRAW;
				break;
			case 2:
				iAnim = STICKYBOMB_WHITE_DRAW;
				break;
			default:
				iAnim = STICKYBOMB_RED_DRAW;
				break;
			}

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_iWeaponState, 1, TRUE, TRUE);

			SendWeaponAnim(iAnim, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;

		switch (m_iWeaponState)
		{
		case 1:
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stickybomb_green_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			iAnim = STICKYBOMB_GREEN_IDLE;
			break;
		case 2:
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stickybomb_white_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			iAnim = STICKYBOMB_WHITE_IDLE;
			break;
		default:
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/stickybomb_red_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			iAnim = STICKYBOMB_RED_IDLE;
			break;
		}

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;
	}
}

BOOL CStickyBomb::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
