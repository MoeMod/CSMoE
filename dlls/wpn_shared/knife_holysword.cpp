/*
knife_holysword.cpp
Copyright (C) 2021 by JustANoobQAQ, Credit Crsky


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif
#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	
#define DEBUG_DRAWBOX 0

	class CHolySwordParray;
	class CKnifeHolySword : public LinkWeaponTemplate<CKnifeHolySword,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_holysword";
		static constexpr const char* V_Model = "models/v_holysword.mdl";
		static constexpr const char* P_Model = "models/p_holysword.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum holysword_e
		{
			ANIM_IDLE,
			ANIM_DRAW,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_SLASH_CHANGE,
			ANIM_CHARGE_START,
			ANIM_CHARGE_LOOP,
			ANIM_CHARGE_SLASH,
			ANIM_CHARGE_SLASH2,
			ANIM_PARING,
			ANIM_PARING_SLASH,
			ANIM_PARING_SLASH2,
			ANIM_FULLSTACK_IDLE,
			ANIM_FULLSTACK_DRAW,
			ANIM_FULLSTACK_SLASH1,
			ANIM_FULLSTACK_SLASH2,
			ANIM_FULLSTACK_SLASH3,
			ANIM_FULLSTACK_CHARGE_START,
			ANIM_FULLSTACK_CHARGE_LOOP,
			ANIM_FULLSTACK_CHARGE_SLASH,
			ANIM_FULLSTACK_CHARGE_SLASH2,
			ANIM_FULLSTACK_PARING,
			ANIM_FULLSTACK_PARING_SLASH,
			ANIM_FULLSTACK_PARING_SLASH2,
		};


		enum holysword_bodygroup_e
		{
			GROUP_HANDS,
			GROUP_STACK,
			GROUP_WEAPON,
			GROUP_WEAPON1,
		};

		void Precache() override
		{
			m_usFireHolySword = PRECACHE_EVENT(1, "events/holysword.sc");

			m_iModelIndex = PRECACHE_MODEL("models/v_holysword.mdl");
			PRECACHE_MODEL("models/p_holysword.mdl");
			PRECACHE_MODEL("models/p_holysword_fullstack.mdl");
			PRECACHE_MODEL("models/p_holysword_parrying.mdl");
			PRECACHE_MODEL("models/p_holysword_parrying2.mdl");
			//PRECACHE_MODEL("models/ef_holystack.mdl");

			PRECACHE_MODEL("sprites/ef_holysword_charge.spr");
			PRECACHE_MODEL("sprites/ef_holysword_stack1.spr");
			PRECACHE_MODEL("sprites/ef_holysword_stack2.spr");
			PRECACHE_MODEL("sprites/ef_holysword_stack3.spr");
			PRECACHE_MODEL("sprites/ef_holysword_stack4.spr");
	
			PRECACHE_SOUND("weapons/holysword_charge_loop.wav");
			PRECACHE_SOUND("weapons/holysword_charge_slash.wav");
			PRECACHE_SOUND("weapons/holysword_paring_slash.wav");
			PRECACHE_SOUND("weapons/holysword_parryattack.wav");
			PRECACHE_SOUND("weapons/holysword_slash1.wav");
			PRECACHE_SOUND("weapons/holysword_slash2.wav");
			PRECACHE_SOUND("weapons/holysword_slash3.wav");
			PRECACHE_SOUND("weapons/holysword_stack_idle.wav");
			PRECACHE_SOUND("weapons/holysword_cannon.wav");

#ifndef CLIENT_DLL
			m_ExpIndex = PRECACHE_MODEL("sprites/muzzleflash78.spr");
			m_Exp2Index = PRECACHE_MODEL("sprites/muzzleflash65.spr");
#endif
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
			m_iState2 = 0;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			m_flTimeBuff = m_tNextThink = m_flTimeCheckCharge = invalid_time_point;
			m_flNextTimeCanHolster = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, false, 0);

			if (m_iStackLevel >= 5)
			{
				SetToBMode();

				DefaultDeploy(V_Model, "models/p_holysword_fullstack.mdl", ANIM_FULLSTACK_DRAW, "sfsword", UseDecrement() != FALSE, 0.1s, 1.0s);
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				return TRUE;
			}
			else
			{
				SetToAMode();

				DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "sfsword", UseDecrement() != FALSE, 0.1s, 1.0s);
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				return TRUE;
			}
			return FALSE;
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void Holster(int skiplocal) override;
		void DelayPrimaryAttack();
		void DelaySecondaryAttack();
		void WeaponCallBack(int iType) override;
#ifndef CLIENT_DLL
		void CreateHitFX(Vector pos, int iAttackType);
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iAttacktype);
		bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, int iAttackType);
		BOOL DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int skiplocal, duration_t fDelay = 0.75s, duration_t flTimeWeaponIdle = 0.75s)
		{
			if (!CanDeploy())
				return FALSE;

			m_pPlayer->TabulateAmmo();
			m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
			m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
			model_name = m_pPlayer->pev->viewmodel;
			Q_strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
			SendWeaponAnim(iAnim, skiplocal);

			m_pPlayer->m_flNextAttack = fDelay;
			m_flTimeWeaponIdle = flTimeWeaponIdle;
			m_flLastFireTime = invalid_time_point;
			m_flDecreaseShotsFired = gpGlobals->time;

			m_pPlayer->m_iFOV = DEFAULT_FOV;
			m_pPlayer->pev->fov = DEFAULT_FOV;
			m_pPlayer->m_iLastZoom = DEFAULT_FOV;
			m_pPlayer->m_bResumeZoom = false;

			return TRUE;
		}
# endif
		BOOL CanDrop() override { return false; }	
		void ChargeAttack();
		void ParingAttack();
		void OnAttackReleased();
		void ItemPostFrame() override;

		BOOL CanHolster() override
		{
			return gpGlobals->time > m_flNextTimeCanHolster;
		}


		void SetToAMode(void)
		{
#ifndef CLIENT_DLL
			pev->modelindex = m_iModelIndex;

			int hands = 0;

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			if (GetBodygroup(GROUP_STACK) == 1)
			{
				SetBodygroup(GROUP_STACK, 0);
			}

			pev->modelindex = 0;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, false, 0);

			if (m_pPlayer && !m_pPlayer->m_bIsZombie)
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_holysword.mdl");
#endif
		}

		void SetToBMode(void)
		{
#ifndef CLIENT_DLL
			pev->modelindex = m_iModelIndex;

			int hands = 1;	//hand 0 seems useless pev->body = 2 male hands with fullstack but make no sense

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			if (GetBodygroup(GROUP_STACK) == 0)
			{
				SetBodygroup(GROUP_STACK, 1);
			}
	
			pev->modelindex = 0;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, false, 0);

			if (m_pPlayer && !m_pPlayer->m_bIsZombie)
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_holysword_fullstack.mdl");
#endif
		}

	public:
		float GetPrimaryAttackDamage(int iState, int iStackLevel) const
		{
			float flDamage;
			switch (iState)
			{
			case 1:
			{
				flDamage = iStackLevel >= 5 ? 130 : 120;
				break;
			}
			case 2:
			{
				flDamage = iStackLevel >= 5 ? 136 : 126;
				break;
			}
			default:
			{
				flDamage = iStackLevel >= 5 ? 190 : 180;
				break;
			}
			}
		
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				switch (iState)
				{
				case 1:
				{
					//flDamage = iStackLevel >= 5 ? 1300 : 1100;
					flDamage = iStackLevel >= 5 ? 900 : 700;
					break;
				}
				case 2:
				{
					//flDamage = iStackLevel >= 5 ? 2000 : 1700;
					flDamage = iStackLevel >= 5 ? 1100 : 900;
					break;
				}
				default:
				{
					//flDamage = iStackLevel >= 5 ? 2600 : 2200;
					flDamage = iStackLevel >= 5 ? 1300 : 1100;
					break;
				}
				}
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				switch (iState)
				{
				case 1:
				{
					flDamage = iStackLevel >= 5 ? 1300 : 1002;
					break;
				}
				case 2:
				{
					flDamage = iStackLevel >= 5 ? 2000 : 1350;
					break;
				}
				default:
				{
					flDamage = iStackLevel >= 5 ? 2600 : 2721;
					break;
				}
				}
			}
#endif
			return flDamage;
		}

		float GetChargeAttackDamage() const
		{
			float flDamage = m_iStackLevel >= 5 ? 170 : 150;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				//flDamage = m_iStackLevel >= 5 ? 6000 : 4600;
				flDamage = m_iStackLevel >= 5 ? 2800 : 2100;
			}	
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = m_iStackLevel >= 5 ? 2090 : 1850;
			}
#endif
			return flDamage;
		}

		float GetSecondaryAttackDamage() const
		{
			float flDamage = m_iStackLevel >= 5 ? 170 : 150;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				//flDamage = m_iStackLevel >= 5 ? 6500 : 5000;
				flDamage = m_iStackLevel >= 5 ? 3000 : 2400;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = m_iStackLevel >= 5 ? 2090 : 1850;
			}
#endif
			return flDamage;
		}
		float GetCannonTouchDamage() const
		{
			float flDamage = m_iStackLevel >= 5 ? 20 : 15;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				//flDamage = m_iStackLevel >= 5 ? 2000 : 1500;
				flDamage = m_iStackLevel >= 5 ? 1300 : 800;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = m_iStackLevel >= 5 ? 2000 : 1500;
			}
#endif
			return flDamage;
		}

		float GetCannonExpDamage() const
		{
			float flDamage = m_iStackLevel >= 5 ? 15 : 10;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				//flDamage = m_iStackLevel >= 5 ? 1400 : 1100;
				flDamage = m_iStackLevel >= 5 ? 900 : 600;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = m_iStackLevel >= 5 ? 1400 : 1100;
			}
#endif
			return flDamage;
		}

		int m_usFireHolySword;
		time_point_t m_flTimeCheckCharge;
		time_point_t m_flTimeBuff;
		int m_iState;
		int m_iState2;
		int m_iStackLevel;
		int m_iModelIndex;
		time_point_t m_tNextThink;
		time_point_t m_flNextTimeCanHolster;
#ifndef CLIENT_DLL
		int m_ExpIndex;
		int m_Exp2Index;
		static TYPEDESCRIPTION m_SaveData[];
		EntityHandle <CHolySwordParray> m_pParray;
#endif
		
	};

#ifndef CLIENT_DLL
	class CHolySwordParray : public CBaseEntity
	{
	public:
		CHolySwordParray()
		{
			m_flRemoveTime = invalid_time_point;
		}

		void Spawn(void);
		void EXPORT OnThink();

		void Init(CBasePlayer* pOwner, CKnifeHolySword* pWeapon);

		void Precache(void)
		{
			m_iModelLight = PRECACHE_MODEL("sprites/laserbeam.spr");
			PRECACHE_MODEL("models/ef_holysword_parryattack2.mdl");
			PRECACHE_MODEL("models/ef_holysword_parryattack.mdl");
		}
#if DEBUG_DRAWBOX
		void DrawBeam();
		time_point_t m_flDrawTime;
#endif

		int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

		int m_iModelLight;
		EntityHandle <CBasePlayer> m_pOwner;
		EntityHandle <CKnifeHolySword> m_pWeapon;
		time_point_t m_flRemoveTime;

		static TYPEDESCRIPTION m_SaveData[];

		void Remove()
		{
			SetThink(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};

	LINK_ENTITY_TO_CLASS(holysword_parray, CHolySwordParray)
#endif
	LINK_ENTITY_TO_CLASS(knife_holysword, CKnifeHolySword)

#ifndef CLIENT_DLL
	TYPEDESCRIPTION CHolySwordParray::m_SaveData[] =
	{
		DEFINE_FIELD(CHolySwordParray, m_pOwner, FIELD_CLASSPTR),
		DEFINE_FIELD(CHolySwordParray, m_pWeapon, FIELD_CLASSPTR),
	};

	TYPEDESCRIPTION CKnifeHolySword::m_SaveData[] =
	{
		DEFINE_FIELD(CKnifeHolySword, m_pParray, FIELD_CLASSPTR),
	};
#endif

	void CKnifeHolySword::Spawn()
	{
		pev->classname = MAKE_STRING("knife_holysword");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_iStackLevel = 0;
		m_flNextTimeCanHolster = m_flTimeBuff = m_tNextThink = m_flTimeCheckCharge = invalid_time_point;
		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		m_iState2 = 0;
		FallInit();
	}

	int CKnifeHolySword::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = NULL;
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

#ifndef CLIENT_DLL
	void CKnifeHolySword::CreateHitFX(Vector pos, int iAttackType)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_SHORT(iAttackType >= 3 ? m_Exp2Index: m_ExpIndex);
		WRITE_BYTE(3);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
#endif
	}


	bool CKnifeHolySword::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, int iAttackType)
	{
		bool bResult = false;
		if (pEntity->IsBSPModel())
		{
			UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, ignore_monsters, pAttacker->edict(), ptr);

			if (ptr->flFraction < 1.0)
			{
				if (ptr->pHit)
				{
					CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

					if (pHit && pHit == pEntity)
						bResult = true;
				}
			}
		}
		else
		{
			UTIL_TraceHull(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, head_hull, pAttacker->edict(), ptr);

			if (ptr->flFraction < 1.0 && ptr->pHit != NULL && CBaseEntity::Instance(ptr->pHit) == pEntity)
			{
				UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, pAttacker->edict(), ptr);

				bResult = true;
			}
			else
			{
				Vector vecEnd = pEntity->BodyTarget(vecSrc);

				if (DotProduct(vecAngleCheckDir, (vecEnd.Make2D() - vecSrc.Make2D()).Normalize()) > flMinimumCosine)
				{
					edict_t* peAttacker = pAttacker->edict();

					for (int i = 5; i > 0; i--)
					{
						UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, peAttacker, ptr);

						if (!ptr->pHit || !GET_PRIVATE(ptr->pHit))
						{
							break;
						}

						CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

						if (ptr->flFraction < 1.0 || pHit == pEntity)
						{
							bResult = true;

							break;
						}

						if (pHit->IsBSPModel())
						{
							break;
						}

						vecSrc = ptr->vecEndPos + vecDir * 5.0;
					}
				}
			}
		}

		//if ((!friendlyfire.value && pEntity->m_iTeam == pAttacker->m_iTeam) || !bResult)
		if (!bResult)
			return bResult;

		if (pEntity->IsPlayer() && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
		{
			CKnifeHolySword::CreateHitFX(pEntity->pev->origin, iAttackType);
		}
		else
		{
			CKnifeHolySword::CreateHitFX(ptr->vecEndPos, iAttackType);
		}
		return bResult;
	}


	hit_result_t CKnifeHolySword::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iAttackType)
	{
		CBaseEntity* pEntity = NULL;
		hit_result_t result = HIT_NONE;
		TraceResult tr;
		UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = pAttackPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity == pAttackPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (bIsWallBetweenEntity(pEntity, pAttackPlayer))
				continue;

			if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}

				if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
				{
					if (!FBitSet(pEntity->pev->flags, FL_DUCKING))
						CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10), iAttackType);
					else
						CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10), iAttackType);
				}
			}

			pAttackPlayer->m_iWeaponVolume = 128;

			ClearMultiDamage();

			pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

			result = HIT_PLAYER;

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
					vecDir.z = 0;
					vecDir = vecDir.Normalize();

					switch (iAttackType)
					{
					case 0:
					{
						//left 3
						ApplyKnockbackData(player, vecDir, { 0, 0, 0, 0, 0.6f });

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.5s);
							strategy->SetStunGravityTime(1.5s);
						}
						break;
					}
					case 1:
					{
						//left 1
						ApplyKnockbackData(player, vecDir, { 0, 0, 0, 0, 0.3f });

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.5s);
							strategy->SetStunGravityTime(1.5s);
						}
						break;

					}
					case 2:
					{
						//left 2
						ApplyKnockbackData(player, vecDir, { 0, 0, 0, 0, 0.3f });

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.5s);
							strategy->SetStunGravityTime(1.5s);
						}
						break;

					}
					case 3:
					{
						//charge
						ApplyKnockbackData(player, vecDir, { 1500, 1000, 1000, 1500, 1.0f });

						if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
						{
							player->pev->velocity.z = 260.0;
						}
						break;
					}
					case 4:
					{
						//parying
						ApplyKnockbackData(player, vecDir, { 1500, 1000, 1000, 1500, 1.0f });

						if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
						{
							player->pev->velocity.z = 260.0;
						}
						break;
					}
					}
				}
			}
		}

		if (!result)
		{
			static int calced = 0;
			static float calcRes = 0.0f;

			if (!(calced & 1))
			{
				calced |= 1;
				calcRes = sqrt(512.0);
			}

			vecEnd = vecSrc + vecForward * (flRadius - calcRes);

			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				vecEnd = vecSrc + vecForward * flRadius;

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

				if (tr.flFraction < 1.0f)
				{
					CreateHitFX(tr.vecEndPos, iAttackType);
				}
			}
				result = HIT_WALL;
		}

		return result;
	}
#endif

	void CKnifeHolySword::DelayPrimaryAttack(void)
	{	
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		
		float flDamage = GetPrimaryAttackDamage(m_iState, m_iStackLevel);
		float flDistance = m_iStackLevel >= 5 ? 130 : 125;
		float flAngle = m_iStackLevel >= 5 ? 180 : 170;

		switch (KnifeAttack(flDamage, flDistance, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, m_iState))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/tomahawk_slash1_hit.wav" : "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			if(m_iStackLevel <= 4)
				m_iStackLevel += 1;

			if(m_iStackLevel == 5)
			{
				m_tNextThink = gpGlobals->time + 0.6s;
			}

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, false, 0);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
#endif

		SetThink(nullptr);
	}


	void CKnifeHolySword::ChargeAttack(void)
	{
#ifndef CLIENT_DLL
		float flDamage = GetChargeAttackDamage();
		float flDistance = m_iStackLevel >= 5 ? 170 : 140;
		float flAngle = m_iStackLevel >= 5 ? 360 : 180;

		switch (KnifeAttack(flDamage, flDistance, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, 3))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/tomahawk_slash1_hit.wav" : "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		CHolySwordCannon* pEnt = static_cast<CHolySwordCannon*>(CBaseEntity::Create("holysword_cannon", vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			float flTouchDamage = GetCannonTouchDamage();
			float flExplodeDamage = GetCannonExpDamage();
			pEnt->Init(gpGlobals->v_forward * 500, flTouchDamage, flExplodeDamage);
		}
#endif

		SetThink(nullptr);
	}

	void CKnifeHolySword::PrimaryAttack(void)
	{
		m_iState2 = 1;
		if (m_iStackLevel == 5)
		{
			m_iStackLevel = 6;
			//m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_holysword_fullstack.mdl");
			SetToBMode();
		}
		m_flTimeCheckCharge = gpGlobals->time + 0.6s;

		switch (m_iState)
		{
			case 0:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				
				m_iState = 1;
				
				SetThink(&CKnifeHolySword::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.2s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;				
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;

#ifndef CLIENT_DLL
				SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_SLASH1 : ANIM_SLASH1, UseDecrement() != FALSE);
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
				
				break;
			}
			case 1:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 2;

				SetThink(&CKnifeHolySword::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.2s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;

#ifndef CLIENT_DLL
				SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_SLASH2 : ANIM_SLASH2, UseDecrement() != FALSE);
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
				break;
			}
			case 2:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 0;

				SetThink(&CKnifeHolySword::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.2s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;

#ifndef CLIENT_DLL
				SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_SLASH3 : ANIM_SLASH3, UseDecrement() != FALSE);
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

				//PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 1, m_iStackLevel, FALSE, FALSE);
			}
		}
	}

	void CKnifeHolySword::ItemPostFrame()
	{
		if (m_flTimeBuff != invalid_time_point && m_flTimeBuff < gpGlobals->time)
		{
			m_pPlayer->m_iBuff &= ~BUFF_HOLYSWORD_AVOID_INFECT;
			m_flTimeBuff = invalid_time_point;
		}

		if (m_flTimeCheckCharge != invalid_time_point && m_flTimeCheckCharge < gpGlobals->time)
		{
			if ((m_pPlayer->pev->button & IN_ATTACK))
			{
				switch (m_iState2)
				{
				case 1:
				{
					if (m_iStackLevel == 5)
					{
						m_iStackLevel = 6;

						SetToBMode();
					}
#ifndef CLIENT_DLL
					SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_CHARGE_START : ANIM_CHARGE_START, UseDecrement() != FALSE);
					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.5, g_vecZero, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, FALSE, TRUE);
#endif
					
					m_iState2 = 2;

					m_flTimeCheckCharge = gpGlobals->time + 0.7s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 9999s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;

					break;
				}
				case 2:
				{
#ifndef CLIENT_DLL
					SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_CHARGE_LOOP : ANIM_CHARGE_LOOP, UseDecrement() != FALSE);
#endif
					
					m_iState2 = 3;
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_charge_loop.wav", VOL_NORM, ATTN_NORM, 0, 94);
					m_flTimeCheckCharge = gpGlobals->time + 3s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;
					break;
				}
				case 3:
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_charge_loop.wav", VOL_NORM, ATTN_NORM, 0, 94);
					m_flTimeCheckCharge = gpGlobals->time + 3s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;
				}
				}
			}
			else
			{
				if(m_iState2 == 2)
					OnAttackReleased();
			}
		}

		if (!(m_pPlayer->pev->button & IN_ATTACK))
		{
			if (m_iState2 == 1)
			{
				m_flTimeCheckCharge = invalid_time_point;
				m_iState2 = 0;
			}			
			else if(m_iState2 == 3)
				OnAttackReleased();
		}
			

		if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time && m_iStackLevel == 5)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(ANIM_SLASH_CHANGE, UseDecrement() != FALSE);
			m_iStackLevel = 6;
#endif		
			SetToBMode();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
			m_tNextThink = invalid_time_point;
			
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeHolySword::WeaponCallBack(int iType)
	{
		ParingAttack();
	}

	void CKnifeHolySword::SecondaryAttack()
	{
		pev->iuser1 = 1;	//enable paring
		m_flTimeBuff = gpGlobals->time + 3.0s;
		m_pPlayer->m_iBuff |= BUFF_HOLYSWORD_AVOID_INFECT;
#ifndef CLIENT_DLL
		if (m_pParray)
			m_pParray->Remove();

		if (m_iStackLevel == 5)
		{
			m_iStackLevel = 6;
			SetToBMode();
		}

		if(m_iStackLevel >= 5)
			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_holysword_parrying2.mdl");
		else
			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_holysword_parrying.mdl");
			

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecForward = gpGlobals->v_forward;
		vecForward.z = 0;
		vecForward = vecForward.Normalize();
		Vector vecAngle = m_pPlayer->pev->angles;
		vecAngle.x = 0;

		m_pParray = static_cast<CHolySwordParray*>(CBaseEntity::Create("holysword_parray", m_pPlayer->pev->origin + vecForward * 40, vecAngle, ENT(m_pPlayer->pev)));
		if (m_pParray)
		{
			m_pParray->Init(m_pPlayer, this);
		}

		if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		{
			strcpy(m_pPlayer->m_szCustomSequence, "crouch_parying_holysword");
		}
		else
		{
			strcpy(m_pPlayer->m_szCustomSequence, "ref_parying_holysword");
		}
		m_pPlayer->pev->frame = 0;
		m_pPlayer->ResetSequenceInfo();
		m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 3.0s;
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireHolySword, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, false, 0);
#endif
		SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_PARING : ANIM_PARING, UseDecrement() != FALSE);
		
		m_iState = 0;
		m_iState2 = 0;
		m_flTimeCheckCharge = invalid_time_point;
		m_flNextTimeCanHolster = gpGlobals->time + 3.0s;
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

	}

	void CKnifeHolySword::OnAttackReleased(void)
	{
#ifndef CLIENT_DLL	
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_CHARGE_SLASH2 : ANIM_CHARGE_SLASH, UseDecrement() != FALSE);

		if (m_iStackLevel >= 5)
		{
			m_iStackLevel = 0;
			//m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
			SetToAMode();
		}
#endif	
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_charge_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/holysword_cannon.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		m_flTimeCheckCharge = invalid_time_point;

		SetThink(&CKnifeHolySword::ChargeAttack);
		pev->nextthink = gpGlobals->time + 0.5s;
		m_iState2 = 0;
		m_iState = 0;
	}

	void CKnifeHolySword::ParingAttack()
	{
		if (pev->iuser1)
		{
#ifndef CLIENT_DLL
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireHolySword, 0.0, m_pPlayer->pev->origin, g_vecZero, 0.0, 0.0, 0, m_iStackLevel, true, 0);

			SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_PARING_SLASH2 : ANIM_PARING_SLASH, UseDecrement() != FALSE);

			if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			{
				strcpy(m_pPlayer->m_szCustomSequence, "crouch_parying_holysword");
			}
			else
			{
				strcpy(m_pPlayer->m_szCustomSequence, "ref_parying_holysword");
			}
			m_pPlayer->pev->frame = 0;
			m_pPlayer->ResetSequenceInfo();
			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.0s;

			if (m_iStackLevel >= 5)
			{
				m_iStackLevel = 0;
				SetToAMode();
			}
#endif
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/holysword_parryattack.wav", VOL_NORM, ATTN_NORM, 0, 94);
			pev->iuser1 = 0;
			SetThink(&CKnifeHolySword::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.5s;
			m_flNextTimeCanHolster = gpGlobals->time + 2.3s;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.3s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		}

	}

	void CKnifeHolySword::DelaySecondaryAttack(void)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/holysword_paring_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
#ifndef CLIENT_DLL	
		float flDamage = GetSecondaryAttackDamage();
		float flRadius = m_iStackLevel >= 5 ? 150 : 140;
		switch (KnifeAttack(flDamage, flRadius, 360, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, 4))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/tomahawk_slash1_hit.wav" : "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
#endif
		SetThink(nullptr);
	}

	float CKnifeHolySword::GetMaxSpeed(void)
	{
			return 250;
	}

	void CKnifeHolySword::Holster(int skiplocal)
	{
		SetThink(nullptr);
		m_iState = 0;
		m_iState2 = 0;
		m_pPlayer->m_iBuff &= ~BUFF_HOLYSWORD_AVOID_INFECT;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeHolySword::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iState = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;		
#ifndef CLIENT_DLL
		SendWeaponAnim(m_iStackLevel >= 5 ? ANIM_FULLSTACK_IDLE : ANIM_IDLE, UseDecrement() != FALSE);
#endif
	}

#ifndef CLIENT_DLL
	void CHolySwordParray::Spawn(void)
	{
		Precache();

		SetThink(&CHolySwordParray::OnThink);
		
		pev->deadflag = DEAD_NO;
		pev->takedamage = DAMAGE_YES;

		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_NOCLIP;

		UTIL_SetSize(pev, Vector(-10.0, -10.0, -80.0), Vector(40.0, 40.0, 64.0));

		m_flRemoveTime = gpGlobals->time + 3.0s;
		pev->nextthink = gpGlobals->time + 0.1s;
#if DEBUG_DRAWBOX
		//SET_MODEL(ENT(pev), "models/summoner.mdl");
		m_flDrawTime = gpGlobals->time + 0.2s;
		DrawBeam();
#endif
	}

	void EXPORT CHolySwordParray::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		if (!m_pOwner || m_pOwner->m_bIsZombie)
		{
			Remove();
		}
		UTIL_MakeVectors(m_pOwner->pev->v_angle);
		
		Vector vecForward = gpGlobals->v_forward;
		vecForward.z = 0;
		vecForward = vecForward.Normalize();
		UTIL_SetOrigin(pev, m_pOwner->pev->origin + vecForward * 40);

		pev->angles = m_pOwner->pev->angles;
		pev->angles.x = 0;

#if DEBUG_DRAWBOX
		if (m_flDrawTime < gpGlobals->time)
		{
			DrawBeam();
			m_flDrawTime = gpGlobals->time + 0.2s;
		}
#endif
		if (m_flRemoveTime < gpGlobals->time)
		{
			Remove();
		}
	}
#if DEBUG_DRAWBOX
	void CHolySwordParray::DrawBeam()
	{
		Vector Point[8];
		Vector Origin = pev->origin;
		
		Point[0] = { pev->maxs.x, pev->maxs.y, pev->maxs.z };
		Point[1] = { pev->maxs.x, pev->maxs.y, pev->mins.z };
		Point[2] = { pev->maxs.x, pev->mins.y, pev->maxs.z };
		Point[3] = { pev->mins.x, pev->maxs.y, pev->maxs.z };
		Point[4] = { pev->mins.x, pev->mins.y, pev->maxs.z };
		Point[5] = { pev->mins.x, pev->maxs.y, pev->mins.z };
		Point[6] = { pev->maxs.x, pev->mins.y, pev->mins.z };
		Point[7] = { pev->mins.x, pev->mins.y, pev->mins.z };

		for (int i = 0; i <= 7; i++)
		{
			Point[i] = Point[i] + Origin;
		}
		//sb
		{
			int life = 2;
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[0].x);
			WRITE_COORD(Point[0].y);
			WRITE_COORD(Point[0].z);
			WRITE_COORD(Point[1].x);
			WRITE_COORD(Point[1].y);
			WRITE_COORD(Point[1].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[0].x);
			WRITE_COORD(Point[0].y);
			WRITE_COORD(Point[0].z);
			WRITE_COORD(Point[3].x);
			WRITE_COORD(Point[3].y);
			WRITE_COORD(Point[3].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[0].x);
			WRITE_COORD(Point[0].y);
			WRITE_COORD(Point[0].z);
			WRITE_COORD(Point[2].x);
			WRITE_COORD(Point[2].y);
			WRITE_COORD(Point[2].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[7].x);
			WRITE_COORD(Point[7].y);
			WRITE_COORD(Point[7].z);
			WRITE_COORD(Point[4].x);
			WRITE_COORD(Point[4].y);
			WRITE_COORD(Point[4].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[7].x);
			WRITE_COORD(Point[7].y);
			WRITE_COORD(Point[7].z);
			WRITE_COORD(Point[5].x);
			WRITE_COORD(Point[5].y);
			WRITE_COORD(Point[5].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[7].x);
			WRITE_COORD(Point[7].y);
			WRITE_COORD(Point[7].z);
			WRITE_COORD(Point[6].x);
			WRITE_COORD(Point[6].y);
			WRITE_COORD(Point[6].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[4].x);
			WRITE_COORD(Point[4].y);
			WRITE_COORD(Point[4].z);
			WRITE_COORD(Point[3].x);
			WRITE_COORD(Point[3].y);
			WRITE_COORD(Point[3].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[4].x);
			WRITE_COORD(Point[4].y);
			WRITE_COORD(Point[4].z);
			WRITE_COORD(Point[2].x);
			WRITE_COORD(Point[2].y);
			WRITE_COORD(Point[2].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[3].x);
			WRITE_COORD(Point[3].y);
			WRITE_COORD(Point[3].z);
			WRITE_COORD(Point[5].x);
			WRITE_COORD(Point[5].y);
			WRITE_COORD(Point[5].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[5].x);
			WRITE_COORD(Point[5].y);
			WRITE_COORD(Point[5].z);
			WRITE_COORD(Point[1].x);
			WRITE_COORD(Point[1].y);
			WRITE_COORD(Point[1].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[1].x);
			WRITE_COORD(Point[1].y);
			WRITE_COORD(Point[1].z);
			WRITE_COORD(Point[6].x);
			WRITE_COORD(Point[6].y);
			WRITE_COORD(Point[6].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[2].x);
			WRITE_COORD(Point[2].y);
			WRITE_COORD(Point[2].z);
			WRITE_COORD(Point[6].x);
			WRITE_COORD(Point[6].y);
			WRITE_COORD(Point[6].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(Point[7].x);
			WRITE_COORD(Point[7].y);
			WRITE_COORD(Point[7].z);
			WRITE_COORD(Point[6].x);
			WRITE_COORD(Point[6].y);
			WRITE_COORD(Point[6].z); // coord coord coord (end position) 
			WRITE_SHORT(m_iModelLight); // short (sprite index) 
			WRITE_BYTE(0); // byte (starting frame) 
			WRITE_BYTE(0); // byte (frame rate in 0.1's) 
			WRITE_BYTE(life); // byte (life in 0.1's) 
			WRITE_BYTE(10); // byte (line width in 0.1's) 
			WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
			WRITE_BYTE(0); // byte,byte,byte (color)
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			WRITE_BYTE(255); // byte (brightness)
			WRITE_BYTE(15); // byte (scroll speed in 0.1's)
			MESSAGE_END();
		}
		
		

	}
#endif

	void CHolySwordParray::Init(CBasePlayer* pOwner, CKnifeHolySword* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;

		pev->health = m_pOwner->pev->health;
	}

	int CHolySwordParray::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
	{
		auto pAttack = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pevAttacker));

		if (!pAttack)
			return 0;

		if (pAttack->IsPlayer() && pAttack->m_bIsZombie && bitsDamageType & DMG_BULLET)
		{
			flDamage = 75.0f;

			if (m_pOwner->pev->health - flDamage >= 300)
				m_pOwner->pev->health -= flDamage;	
		}

		if (m_pWeapon && g_pGameRules->PlayerRelationship(m_pOwner, pAttack) != GR_TEAMMATE)
		{
			m_pWeapon->WeaponCallBack(0);
		}
			
		flDamage = 0.0f;

		return CBaseEntity::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

#endif
}
