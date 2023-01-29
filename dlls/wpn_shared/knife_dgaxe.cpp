/*
knife_dgaxe.cpp
Copyright (C) 2019 JustANoobQAQ

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


enum summon_anim
{
	ATTACK1_START,
	ATTACK1_END,
	ATTACK2_START,
	ATTACK2_LOOP,
	ATTACK2_END,
};

#ifndef CLIENT_DLL
namespace sv {
	class CDgaxeSummon : public CBaseEntity
	{
	public:
		float GetArmorRatioModifier() { return ::sv::GetArmorRatioModifier(WEAPON_KNIFE); }
		int GetWeaponsId(void) { return WEAPON_KNIFE; }
		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/dgaxe_summon.mdl");
			pev->animtime = gpGlobals->time;
			pev->sequence = 0;
			pev->framerate = 1.0;
			pev->solid = SOLID_BBOX; // 2	
			pev->rendermode = kRenderTransTexture; // 5;
			//pev->renderfx = kRenderFxNone;
			pev->renderamt = 170.0;
			pev->movetype = MOVETYPE_FLY; // 9
			SetThink(&CDgaxeSummon::Skill);
			pev->nextthink = gpGlobals->time + 0.01s;
			pev->classname = MAKE_STRING("dgaxe_summon");
			m_flDamageTime = invalid_time_point;
			m_flLoopTime = invalid_time_point;
		}

		void Skill()
		{
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 4, 1, TRUE, FALSE);
			if (m_iSkill == 1)
				SetThink(&CDgaxeSummon::Skill2OnStart);	//tuoluoren
			else
				SetThink(&CDgaxeSummon::Skill1OnStart);
			pev->nextthink = gpGlobals->time + 0.01s;
		}

		void Skill1OnStart()
		{
			pev->movetype = MOVETYPE_TOSS;
			pev->sequence = ATTACK1_START;
			pev->animtime = gpGlobals->time;
			pev->framerate = 1.0;
			m_iState = ATTACK1_START;
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill1.wav", VOL_NORM, ATTN_NORM);

			SetThink(&CDgaxeSummon::Skill1OnEnd);
			pev->nextthink = gpGlobals->time + 0.36s;
		}

		void Skill1OnEnd()
		{
			if (m_iState == ATTACK1_START)
			{
				pev->sequence = ATTACK1_END;
				pev->animtime = gpGlobals->time;
				pev->framerate = 1.0;
				m_iState = ATTACK2_END;
				m_freq = 40;
				m_flLoopTime = gpGlobals->time + 1.7s;
				m_flDamageTime = gpGlobals->time + 0.2s;
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill1_exp.wav", VOL_NORM, ATTN_NORM);

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(pev->origin[0]);
				WRITE_COORD(pev->origin[1]);
				WRITE_COORD(pev->origin[2]);
				WRITE_SHORT(MODEL_INDEX("sprites/ef_dgaxe_explo.spr"));
				WRITE_BYTE(15);
				WRITE_BYTE(30);
				WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
				MESSAGE_END();

			}

			if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
			{
				RadiusDamage(m_SkillDamage, TRUE);
				m_flDamageTime = invalid_time_point;
			}

			pev->renderamt -= 1;
			pev->nextthink = gpGlobals->time + 0.01s;
			if (gpGlobals->time > m_flLoopTime)		//after 1.7s
			{
				SetThink(&CDgaxeSummon::Remove);
				pev->nextthink = gpGlobals->time + 0.01s;
			}
		}

		void Skill2OnStart()
		{
			pev->sequence = ATTACK2_START;
			pev->animtime = gpGlobals->time;
			pev->framerate = 1.0;
			m_iState = ATTACK2_START;
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill2.wav", VOL_NORM, ATTN_NORM);

			SetThink(&CDgaxeSummon::Skill2OnLoop);
			pev->nextthink = gpGlobals->time + 0.36s;
			m_flDamageTime = gpGlobals->time + 0.25s;
		}

		void Skill2OnLoop()
		{
			if (m_iState == ATTACK2_START)
			{
				m_iState = ATTACK2_LOOP;
				pev->sequence = ATTACK2_LOOP;
				pev->animtime = gpGlobals->time;
				pev->framerate = 1.0;
				m_flLoopTime = gpGlobals->time + 5.0s;
				
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill2_exp.wav", VOL_NORM, ATTN_NORM);
			}
			if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
			{
				RadiusDamage(m_SkillDamage, FALSE);
				m_flDamageTime = gpGlobals->time + 0.25s;
			}
			
			pev->nextthink = gpGlobals->time + 0.01s;
			//RadiusDamage();
			if (gpGlobals->time > m_flLoopTime)		//after 4s
			{
				SetThink(&CDgaxeSummon::Skill2OnEnd);
				pev->nextthink = gpGlobals->time + 0.01s;
				m_flDamageTime = invalid_time_point;
			}

		}

		void Skill2OnEnd()
		{
			if (m_iState == ATTACK2_LOOP)
			{
				pev->sequence = ATTACK2_END;
				pev->animtime = gpGlobals->time;
				pev->framerate = 1.0;
				m_iState = ATTACK2_END;
				m_flLoopTime = gpGlobals->time + 0.33s;
			}
			pev->renderamt -= 4;
			pev->nextthink = gpGlobals->time + 0.01s;
			if (gpGlobals->time > m_flLoopTime)		//after 0.33s
			{
				SetThink(&CDgaxeSummon::Remove);
				pev->nextthink = gpGlobals->time + 0.01s;
			}

		}

		void RadiusDamage(float flDamage, bool bIsSkill1)
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

			if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
			{
				TraceResult tr;
				CBaseEntity* pEntity = NULL;

				Vector vecEnd;
				int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 175.0)) != NULL)
				{
					if (pEntity->edict() == pev->owner)
						continue;

					if (pEntity->IsDormant())
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity->Classify() == CLASS_NONE)
						continue;

					if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
						continue;

					if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
						continue;

					if (pEntity->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (!player)
							continue;

						vecEnd = player->BodyTarget(pev->origin);

						UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

						if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
							continue;

						player->TakeDamage(this->pev, pAttackPlayer->pev, flDamage, DMG_BULLET | DMG_NEVERGIB);

						if (player->m_bIsZombie && ~(player->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
						{
							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
							if (strategy)
							{
								if (bIsSkill1)
								{
									strategy->SetStunSpeedTime(1.0s, 150.0f);	//150
									strategy->SetStunGravityTime(1.0s);
								}
								else
								{
									strategy->SetStunSpeedTime(2.0s, 60.0f);	//60
									strategy->SetStunGravityTime(2.0s);
								}

							}
						}					
					}
					else
					{
						vecEnd = pEntity->BodyTarget(pev->origin);

						UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

						if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
							continue;

						pEntity->TakeDamage(this->pev, pAttackPlayer->pev, flDamage, DMG_BULLET | DMG_NEVERGIB);
					}			
				}
			}
		}

		void Init(float flDamage, int eventindex, int iSkill, CBasePlayer* pPlayer)
		{
			m_SkillDamage = flDamage;
			m_usDgaxe = eventindex;
			m_pPlayer = pPlayer;
			m_iSkill = iSkill;
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/ef_dgaxe_explo.spr");	
			PRECACHE_MODEL("sprites/ef_dgaxe_smoke.spr");
			PRECACHE_MODEL("models/dgaxe_summon.mdl");	
		}

		float m_SkillDamage;
		int m_usDgaxe;
		int m_freq;
		int m_iSkill;
		CBasePlayer* m_pPlayer;
		int m_iState;
		time_point_t m_flLoopTime;
		time_point_t m_flDamageTime;

		protected:
			void Remove()
			{
				SetThink(nullptr);
				pev->effects |= EF_NODRAW; // 0x80u

				return UTIL_Remove(this);
			}		
	};

	LINK_ENTITY_TO_CLASS(dgaxe_summon, CDgaxeSummon);
}
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif



#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

	class CKnifeDgaxe : public LinkWeaponTemplate<CKnifeDgaxe,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_dgaxe";
		static constexpr const char* V_Model = "models/v_dgaxe.mdl";
		static constexpr const char* V_Model_1 = "models/v_dgaxe_1.mdl";
		static constexpr const char* P_Model = "models/p_dgaxe_a.mdl";
		static constexpr const char* P_Model_B = "models/p_dgaxe_b.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = 100;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "knife";
		KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

	public:

		enum dgaxe_e
		{
			DGAXE_IDLE,
			DGAXE_IDLE2,
			DGAXE_DRAW,
			DGAXE_SLASH1,
			DGAXE_SLASH2,
			DGAXE_SLASH3,
			DGAXE_SLASH4,
			DGAXE_DUALSLASH1,
			DGAXE_DUALSLASH2,
			DGAXE_COMBO,
			DGAXE_SKILL1,
			DGAXE_SKILL2
		};

		enum damagetype_e
		{
			DGAXE_PRIMARYATTACK,
			DGAXE_SECONDARYATTACK,
			DGAXE_COMBOATTACK,
			DGAXE_SKILL1ATTACK,
			DGAXE_SKILL2ATTACK,
		};

		enum crosshairstate_e
		{
			DGAXE_AIM_OFF,
			DGAXE_AIM_ON,
			DGAXE_AIM_ON_2,
			DGAXE_AIM_NO,
		};

		enum combo_e
		{
			DGAXE_COMBO_IDLE,
			DGAXE_COMBO_RIGHT,
			DGAXE_COMBO_LEFT1,
			DGAXE_COMBO_LEFT2,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_dgaxe.mdl");
			PRECACHE_MODEL("models/v_dgaxe_1.mdl");
			PRECACHE_MODEL("models/p_dgaxe_a.mdl");
			PRECACHE_MODEL("models/p_dgaxe_b.mdl");
#ifndef CLIENT_DLL
			PRECACHE_MODEL("sprites/muzzleflash200.spr");
			PRECACHE_MODEL("sprites/muzzleflash201.spr");
			PRECACHE_MODEL("sprites/muzzleflash202.spr");
			PRECACHE_MODEL("sprites/muzzleflash203.spr");
			PRECACHE_MODEL("sprites/muzzleflash204.spr");
			PRECACHE_MODEL("sprites/ef_dgaxe_change.spr");
#endif
			
				
			PRECACHE_SOUND("weapons/dgaxe_combo.wav");
			PRECACHE_SOUND("weapons/dgaxe_ready.wav");

			PRECACHE_SOUND("weapons/dgaxe_skill1_exp.wav"); //zhongpi
			PRECACHE_SOUND("weapons/dgaxe_skill2_exp.wav"); //tuoluo

			PRECACHE_SOUND("weapons/dgaxe_dualslash1.wav"); //left
			PRECACHE_SOUND("weapons/dgaxe_dualslash2.wav");

			PRECACHE_SOUND("weapons/dgaxe_slash1.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash2.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash3.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash4.wav");

			PRECACHE_SOUND("weapons/dgaxe_slash_hit1.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash_hit2.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash_hit3.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_metal1.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_metal2.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_stone1.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_stone2.wav");

			m_usDgaxe = PRECACHE_EVENT(1, "events/effect.sc");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 260;
			m_iComboState = DGAXE_COMBO_IDLE;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			m_pPlayer->m_bShieldDrawn = false;
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
			WRITE_BYTE(0);
			WRITE_BYTE(m_CrossHairState);
			MESSAGE_END();
#endif
			if (m_flTimeHostler != invalid_time_point)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 0.33s;
			}
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 100;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50 && m_iHolsterAmmo < 50)
			{
				EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "weapons/dgaxe_ready.wav", VOL_NORM, ATTN_NORM, 0, 94);
				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 4, 2, TRUE, FALSE);
			}
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_draw.wav", 0.3, 2.4);
			return DefaultDeploy(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50 ? V_Model_1 : V_Model, P_Model, DGAXE_DRAW, "tomahawk", UseDecrement() != FALSE, 0.1s);
		}

		int GetItemInfo(ItemInfo* p) override;
		float GetMaxSpeed() override { return 260; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void Spawn() override;
		void Holster(int skiplocal) override;
		void WeaponIdle() override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		void ComboThink();
		void ComboAttack();
		void Guard();
		KnockbackData GetKnockBackData() override;
		BOOL CanDrop() override { return false; }
		BOOL IsSignal();
		BOOL CanHolster() override;
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
		Vector Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, int* bHit);
		void ItemPostFrame() override;
#endif

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 90;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 2400;
				else
					flDamage *= 17.5f;
			}
				
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10380.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 45;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 400;
				else
					flDamage *= 7.0f;
			}
			
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1665.0f;
#endif
			return flDamage;
		}
		float Skill1AttackDamage() const
		{
			float flDamage = 70;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 14700;
				else
					flDamage *= 100.5f;
			}
				
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 20425.0f;
#endif
			return flDamage;
		}
		float Skill2AttackDamage() const
		{
			float flDamage = 45;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 761;
				else
					flDamage *= 10.5f;
			}		
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1200.0f;
#endif
			return flDamage;
		}
		float ComboAttackDamage() const
		{
			float flDamage = 95;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 4350;
				else
					flDamage *= 33.5f;
			}		
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 20400.0f;
#endif
			return flDamage;
		}


		time_point_t m_flTimeNextCharge;
		time_point_t m_flTimeNextSkill1;
		time_point_t m_flTimeNextSkill2;
		time_point_t m_flTimeHostler;
		time_point_t m_flTimeNextMessage;
		int m_iComboState;
		int m_iHolsterAmmo;
		int m_usDgaxe;
		int m_CrossHairState;
		int m_CrossHairStateStored;
		int SlashAnim;
		int DualSlashAnim;	
		int m_iButton;
		bool bThinking;
	};

	LINK_ENTITY_TO_CLASS(knife_dgaxe, CKnifeDgaxe)

	void CKnifeDgaxe::Spawn()
	{
		pev->classname = MAKE_STRING("knife_dgaxe");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_iClip = -1;
		m_flTimeNextMessage = invalid_time_point;
		m_iComboState = DGAXE_COMBO_IDLE;
		m_flTimeHostler = gpGlobals->time;
		m_iDefaultAmmo = 0;
		SlashAnim = 3;
		DualSlashAnim = 7;
		m_CrossHairState = m_CrossHairStateStored = DGAXE_AIM_NO;
		FallInit();
	}

	int CKnifeDgaxe::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "dgaxeammo";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 100;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	KnockbackData CKnifeDgaxe::GetKnockBackData() { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

#ifndef CLIENT_DLL
	hit_result_t CKnifeDgaxe::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
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

			if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, pAttackPlayer))
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
					if (IsPrimaryAttack)
					{
						if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(2.0s);
								strategy->SetStunGravityTime(2.0s);
							}
						}
					}
					else
					{
						Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.6f });
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
				result = HIT_WALL;
		}

		return result;
	}
#endif

	void CKnifeDgaxe::PrimaryAttack(void)
	{
		if (m_iComboState == DGAXE_COMBO_RIGHT)
			m_iComboState = DGAXE_COMBO_LEFT1;
		else if (m_iComboState == DGAXE_COMBO_LEFT1)
		{
			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_B);
			Q_strcpy(m_pPlayer->m_szAnimExtention, "hammer");
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(DGAXE_COMBO, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_STATIC, "weapons/dgaxe_combo.wav", VOL_NORM, ATTN_NORM, 0, 94);
			SetThink(&CKnifeDgaxe::ComboAttack);
			pev->nextthink = gpGlobals->time + 0.73s;

			m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.51s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
			m_iComboState = DGAXE_COMBO_IDLE;
			currentammo = 1;
			return;
		}
		else
			m_iComboState = DGAXE_COMBO_IDLE;

		if (DualSlashAnim > 8)
			DualSlashAnim = 7;
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 360, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_NONE:
		{
			SendWeaponAnim(DualSlashAnim, UseDecrement() != FALSE);
			if (DualSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_dualslash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_dualslash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			SendWeaponAnim(DualSlashAnim, UseDecrement() != FALSE);
			if (DualSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			SendWeaponAnim(DualSlashAnim, UseDecrement() != FALSE);
			if (SlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_wall_stone2.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
		}
#endif
		DualSlashAnim++;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.97s;
		SlashAnim++;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
	}

	int CKnifeDgaxe::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void CKnifeDgaxe::SecondaryAttack(void)
	{
		if (!m_iComboState)
			m_iComboState = DGAXE_COMBO_RIGHT;
		if (SlashAnim >= 7)
			SlashAnim = 3;
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetSecondaryAttackDamage(), 180, 180, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_NONE:
		{
			SendWeaponAnim(SlashAnim, UseDecrement() != FALSE);
			switch (SlashAnim)
			{
			case 3:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94); break;
			case 4:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94); break;
			case 5:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash3.wav", VOL_NORM, ATTN_NORM, 0, 94); break;
			case 6:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash4.wav", VOL_NORM, ATTN_NORM, 0, 94); break;
			}
			break;
		}
		case HIT_PLAYER:
		{
			SendWeaponAnim(SlashAnim, UseDecrement() != FALSE);
			if (SlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			SendWeaponAnim(SlashAnim, UseDecrement() != FALSE);
			if (SlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_wall_stone2.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
		}
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		SlashAnim++;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
	}

	void CKnifeDgaxe::ComboAttack()
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 220;

		SetThink(&CKnifeDgaxe::Guard);
		pev->nextthink = gpGlobals->time + 1.27s;
		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");
#ifndef CLIENT_DLL
		currentammo = 0;
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 4, 0, TRUE, TRUE);
		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction >= 1.0)
			{
				return;
			}

			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
			{
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));
			}

			vecEnd = tr.vecEndPos;
		}

		if (tr.flFraction < 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (!pEntity || pEntity->IsBSPModel())
			{
				ClearMultiDamage();
				pEntity->TraceAttack(m_pPlayer->pev, ComboAttackDamage(), gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}		
		}

		CBaseEntity* pEntity = nullptr;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 115)) != nullptr) {
			if (pEntity->pev->takedamage != DAMAGE_NO) {
				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->pev == m_pPlayer->pev)
					continue;

				Vector vecSpot = pEntity->BodyTarget(vecSrc);
				vecSpot.z = vecEnd.z;
				UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(m_pPlayer->pev), &tr);

				if (AngleBetweenVectors(tr.vecEndPos - vecSrc, gpGlobals->v_forward) > 120)
					continue;

				if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict()) {
					if (tr.fStartSolid) {
						tr.vecEndPos = vecSrc;
						tr.flFraction = 0;
					}

					if (tr.flFraction == 1.0f) {
						pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, ComboAttackDamage(), DMG_BULLET | DMG_NEVERGIB);
					}

					Vector vecRealDir = (tr.vecEndPos - vecSrc).Normalize();

					ClearMultiDamage();
					pEntity->TraceAttack(m_pPlayer->pev, ComboAttackDamage(), vecRealDir, &tr, DMG_BULLET | DMG_NEVERGIB);
					ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

					CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
					if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
					{
						pPlayer->m_flVelocityModifier = 1.0;
						if (!(pPlayer->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							Vector2D vecDir2D = (pPlayer->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

							pPlayer->pev->velocity.x = vecDir2D.x * 1000;
							pPlayer->pev->velocity.y = vecDir2D.y * 1000;
							pPlayer->pev->velocity.z = 600;
						}
					}

					if (pEntity->Classify() == CLASS_PLAYER)
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
					else
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
		}
#endif
	}

	void CKnifeDgaxe::Guard()
	{
		if (m_pPlayer->m_iBuff & BUFF_AVOID_INFECT)
			m_pPlayer->m_iBuff &= ~BUFF_AVOID_INFECT;
		SetThink(nullptr);
	}

	void CKnifeDgaxe::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iComboState = DGAXE_COMBO_IDLE;

		if (RANDOM_LONG(0, 5))
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8s;
			SendWeaponAnim(DGAXE_IDLE, UseDecrement() != FALSE);
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
			SendWeaponAnim(DGAXE_IDLE2, UseDecrement() != FALSE);
		}
		
	}


	BOOL CKnifeDgaxe::IsSignal()
	{
#ifndef CLIENT_DLL
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50)
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_dgaxe_1.mdl");
			return TRUE;
		}
		else
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_dgaxe.mdl");
			m_CrossHairState = DGAXE_AIM_NO;
			return FALSE;
		}
#endif
		return TRUE;
	}


	void CKnifeDgaxe::Holster(int skiplocal)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
		WRITE_BYTE(0);
		WRITE_BYTE(3);
		MESSAGE_END();
#endif
		SetThink(nullptr);
		m_pPlayer->m_iBuff &= ~BUFF_AVOID_INFECT;
		m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
		m_flTimeHostler = gpGlobals->time;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

#ifndef CLIENT_DLL
	Vector CKnifeDgaxe::Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, int* bHit = 0)
	{
		Vector end;

		end = pevAttacker->pev->v_angle;
		UTIL_MakeVectors(end);
		end = gpGlobals->v_forward;
		end = end * 8192.0;
		end = Start + end;

		TraceResult tr;
		UTIL_TraceLine(Start, end, dont_ignore_monsters, pevAttacker->edict(), &tr);
		end = tr.vecEndPos;
		if (tr.flFraction != 1.0f)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			{
				if (pEntity->pev->takedamage != DAMAGE_NO)
				{
					if (pEntity->IsBSPModel())
						return end;

					if (pEntity->IsPlayer() && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
						return end;

					*bHit = 1;
				}
			}
		}
		return end;
	}

	void CKnifeDgaxe::ItemPostFrame()
	{
		m_iButton = m_pPlayer->pev->button;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
				m_flTimeNextCharge = gpGlobals->time + 0.3s;

				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 50)
				{
					EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "weapons/dgaxe_ready.wav", VOL_NORM, ATTN_NORM, 0, 94);
					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 4, 2, TRUE, FALSE);
				}
			}
		}
		if (IsSignal())
		{
			m_CrossHairState = DGAXE_AIM_OFF;
			if (m_iButton & IN_ATTACK)
				m_CrossHairState = DGAXE_AIM_ON_2;
		}
		int bHit = 0;
		Vector vecSrc = m_pPlayer->GetGunPosition();
		CBaseEntity* pevAttacker = this->m_pPlayer;
		auto vecShootPosition = Get_ShootPosition(pevAttacker, vecSrc, &bHit);
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50)
		{
			if (bHit)
			{
				m_CrossHairState = DGAXE_AIM_ON;
				if (m_iButton & IN_RELOAD)
				{
					if (m_flTimeNextSkill2 < gpGlobals->time)
					{
						m_pPlayer->SetAnimation(PLAYER_ATTACK1);
						SendWeaponAnim(DGAXE_SKILL1, UseDecrement() != FALSE);
						m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
						SetThink(&CKnifeDgaxe::Guard);
						pev->nextthink = gpGlobals->time + 2.0s;
						CDgaxeSummon* pEnt = static_cast<CDgaxeSummon*>(CBaseEntity::Create("dgaxe_summon", vecShootPosition, { 0,0,0 }, ENT(m_pPlayer->pev)));
						if (pEnt)
						{
							pEnt->Init(Skill1AttackDamage(), m_usDgaxe, 0, m_pPlayer);
						}
						m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 40;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.51s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
						m_flTimeNextSkill2 = gpGlobals->time + 3.0s;
					}
				}
				if (m_iButton & IN_ATTACK)
					m_CrossHairState = DGAXE_AIM_ON_2;
			}
			else
				if (m_iButton & IN_RELOAD && m_iButton & IN_ATTACK)
				{
					if (m_flTimeNextSkill1 < gpGlobals->time)
					{
						m_pPlayer->SetAnimation(PLAYER_ATTACK1);
						SendWeaponAnim(DGAXE_SKILL2, UseDecrement() != FALSE);
						m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
						SetThink(&CKnifeDgaxe::Guard);
						pev->nextthink = gpGlobals->time + 2.0s;
						CDgaxeSummon* pEnt = static_cast<CDgaxeSummon*>(CBaseEntity::Create("dgaxe_summon", vecShootPosition, { 0,0,0 }, ENT(m_pPlayer->pev)));
						if (pEnt)
						{
							pEnt->Init(Skill2AttackDamage(), m_usDgaxe, 1, m_pPlayer);
						}
						m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 50;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.51s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
						m_flTimeNextSkill1 = gpGlobals->time + 8.0s;
					}

				}
		}

		if (m_flTimeNextMessage < gpGlobals->time)
		{
			if (m_CrossHairState != m_CrossHairStateStored)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(0);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();
			}

			MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgAmmoX, NULL, m_pPlayer->pev);
			WRITE_BYTE(m_iPrimaryAmmoType);
			WRITE_SHORT(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
			MESSAGE_END();

			m_flTimeNextMessage = gpGlobals->time + 0.1s;
			m_CrossHairStateStored = m_CrossHairState;
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}
#endif

	BOOL CKnifeDgaxe::CanHolster(void)
	{
#ifndef CLIENT_DLL
		if (currentammo)
			return FALSE;
		else
			return TRUE;
#endif
		return FALSE;
	}
}
