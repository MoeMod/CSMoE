/*
knife_lance.cpp
Copyright (C) 2021 Original Author ���դξ��� Modify by JustANoobQAQ , Entity Think by Rentro


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

#ifndef CLIENT_DLL
namespace sv {
	class CLanceFountainFieldAttack : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/ef_lance_fieldattack.mdl");

			pev->animtime = gpGlobals->time;
			pev->sequence = 0;
			pev->framerate = 1.0;
			pev->frame = 0.0;
			pev->solid = SOLID_TRIGGER; // 2	
			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderamt = 255.0;
			pev->movetype = MOVETYPE_NONE; // 9
			UTIL_SetSize(pev, { -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });
			pev->nextthink = gpGlobals->time + 0.01s;
			SetThink(&CLanceFountainFieldAttack::OnThink);
			pev->classname = MAKE_STRING("lance");

			m_tTimeRemove = gpGlobals->time + 3s;

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/lance_field_attack_inoperation.wav", VOL_NORM, ATTN_NORM, 0, 94);
		}

		void OnThink()
		{
			pev->nextthink = gpGlobals->time + 0.2s;

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			CBasePlayer* pInflictorPlayer = nullptr;

			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackePlayer)
			{
				SetThink(&CBaseEntity::SUB_StartFadeOut);
				return;
			}

			if (!pAttackePlayer->IsAlive())
			{
				SetThink(&CBaseEntity::SUB_StartFadeOut);
				return;
			}

			if (pAttackePlayer->m_bIsZombie)
			{
				SetThink(&CBaseEntity::SUB_StartFadeOut);
				return;
			}

			RadiusDamage();

			if (m_tTimeRemove < gpGlobals->time)
			{
				pev->effects |= EF_NODRAW; // 0x80u
				SetThink(NULL);
				UTIL_Remove(this);
			}
		}

		void RadiusDamage()
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

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 250.0)) != NULL)
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

					if (pev->origin.z - pEntity->pev->origin.z > 40)
						continue;

					if (pEntity->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (!player)
							continue;

						if (pAttackPlayer->m_iTeam == player->m_iTeam)
							continue;

						player->TakeDamage(pAttackPlayer->pev, pAttackPlayer->pev, m_SkillDamage, DMG_BULLET | DMG_NEVERGIB);

						if (player->m_bIsZombie)
						{
							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(0.2s);
								strategy->SetStunGravityTime(0.2s);
							}
						}

						MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, player->pev);
						WRITE_BYTE(9);	//type
						WRITE_SHORT(ENTINDEX(player->edict()));	//entity
						WRITE_BYTE(1);
						MESSAGE_END();
					}
					else
					{
						pEntity->TakeDamage(pAttackPlayer->pev, pAttackPlayer->pev, m_SkillDamage, DMG_BULLET | DMG_NEVERGIB);
					}
				}
			}
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_lance_fieldattack.mdl");

			PRECACHE_SOUND("weapons/lance_field_attack_inoperation.wav");
		}

		void Init(float flDamage, int iSkill, CBasePlayer* pPlayer)
		{
			m_SkillDamage = flDamage;
			m_pPlayer = pPlayer;
			m_iSkill = iSkill;
		}

		float m_SkillDamage;
		CBasePlayer* m_pPlayer;
		time_point_t m_tTimeRemove;
		int m_iSkill;
	};

	LINK_ENTITY_TO_CLASS(lance_fountainfield, CLanceFountainFieldAttack);

	class CLanceFountainSpecialAttack : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();
			//11223
			if(pev->iuser1 == 1 || pev->iuser1 == 2)
				SET_MODEL(this->edict(), "models/ef_lance_specialattack1.mdl");
			else if(pev->iuser1 == 3 || pev->iuser1 == 4)
				SET_MODEL(this->edict(), "models/ef_lance_specialattack2.mdl");
			else if (pev->iuser1 == 5)
				SET_MODEL(this->edict(), "models/ef_lance_specialattack3.mdl");
			else
				SET_MODEL(this->edict(), "models/ef_lance_specialattack1.mdl");

			pev->animtime = gpGlobals->time;
			pev->sequence = 0;
			pev->framerate = 1.0;
			pev->frame = 0.0;
			pev->solid = SOLID_NOT; // 2	
			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderamt = 255.0;
			pev->movetype = MOVETYPE_NONE; // 9
			UTIL_SetSize(pev, { -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });		
			SetThink(&CLanceFountainSpecialAttack::OnThink);
			pev->classname = MAKE_STRING("lance");
			m_iDoDamage = 0;
			EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/lance_special_attack_inoperation.wav", VOL_NORM, ATTN_NORM);

			pev->nextthink = gpGlobals->time + 0.01s;
			m_tTimeRemove = gpGlobals->time + 0.2s;
		}

		static CLanceFountainSpecialAttack* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iType)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}
			CBaseEntity* pEntity = Instance(pent);
			CLanceFountainSpecialAttack* pEnt = static_cast<CLanceFountainSpecialAttack*>(pEntity);
			pEnt->pev->owner = pentOwner;
			pEnt->pev->origin = vecOrigin;
			pEnt->pev->angles = vecAngles;
			pEnt->pev->iuser1 = iType;
			DispatchSpawn(pEnt->edict());		
			return pEnt;
		}

		void OnThink()
		{
			pev->animtime = gpGlobals->time;
			pev->nextthink = gpGlobals->time + 0.5s;
			if (!m_iDoDamage)
			{
				CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
				CBasePlayer* pAttackePlayer = nullptr;
				CBasePlayer* pInflictorPlayer = nullptr;

				if (pAttacker && pAttacker->IsPlayer())
					pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

				if (!pAttackePlayer)
				{
					SetThink(&CBaseEntity::SUB_StartFadeOut);
					return;
				}

				if (!pAttackePlayer->IsAlive())
				{
					SetThink(&CBaseEntity::SUB_StartFadeOut);
					return;
				}

				if (pAttackePlayer->m_bIsZombie)
				{
					SetThink(&CBaseEntity::SUB_StartFadeOut);
					return;
				}

				Vector vecOrigin = pev->origin;
				vecOrigin[2] += 250;
				RadiusDamage();
				m_iDoDamage = 1;
			}

			if (m_tTimeRemove <= gpGlobals->time)
			{
				switch (pev->iuser1)
				{
				case 1:
				{
					pev->iuser1 = 2;
					break;
				}
				case 2:
				{
					pev->iuser1 = 3;
					break;
				}
				case 3:
				{
					pev->iuser1 = 4;
					break;
				}
				case 4:
				{
					pev->iuser1 = 5;
					break;
				}
				case 5:
					pev->iuser1 = 0;
					break;
				}
				if (pev->iuser1)
				{
					//��һ�����ӵ�ǰλ��ˮƽ��ǰ����80����ס���λ��Ϊtemp(vecOrigin)	By Rentro
					//�ڶ�������temp�����Ϸ���traceline��������50��endpos��Ϊtemp2 (VecEnd1)
					//����������temp2���·�traceline�������ǣ�temp2.z - temp.z) + 50
					//endpos��Ϊtemp3
					//���Ĳ��������һ��traceline��startsolid��true��
					//��ôѡ��temp
					//����ѡ��temp3

					Vector vecOrigin = pev->origin;
					vecOrigin += m_vecDir * 80;
					TraceResult tr;
					Vector vecEnd;

					vecEnd = vecOrigin;
					float fltemp = vecEnd.z - 48.0f;
					vecEnd.z += 48.0f;

					UTIL_TraceLine(vecOrigin, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
					vecEnd.z = fltemp;
					UTIL_TraceLine(tr.vecEndPos, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
					if (!tr.fStartSolid)
						vecOrigin = tr.vecEndPos;

					CLanceFountainSpecialAttack* pEnt = CLanceFountainSpecialAttack::Create("lance_fountainspecial", vecOrigin, { 0,0,0 }, pev->owner, pev->iuser1);
					if (pEnt)
					{
						pEnt->Init(m_SkillDamage, 0, m_vecDir);
					}
				}

				SetThink(&CLanceFountainSpecialAttack::StartFadeOut);
				pev->nextthink = gpGlobals->time + 0.1s;
				return;
			}
		}

		void RadiusDamage()
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

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 200.0)) != NULL)
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

					if (pev->origin.z - pEntity->pev->origin.z > 40)
						continue;

					if (pEntity->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (!player)
							continue;

						if (pAttackPlayer->m_iTeam == player->m_iTeam)
							continue;

						player->TakeDamage(pAttackPlayer->pev, pAttackPlayer->pev, m_SkillDamage, DMG_BULLET | DMG_NEVERGIB);

						if (player->m_bIsZombie && player->m_ZombieClass)
						{
							if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
							{
								Vector2D vecDir = (player->pev->origin - pAttackPlayer->pev->origin).Make2D().Normalize();

								player->pev->velocity.x = vecDir.x * 600.0;
								player->pev->velocity.y = vecDir.y * 600.0;
								player->pev->velocity.z = 310.0;
							}					
						}

						MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, player->pev);
						WRITE_BYTE(9);	//type
						WRITE_SHORT(ENTINDEX(player->edict()));	//entity
						WRITE_BYTE(1);
						MESSAGE_END();
					}
					else
					{
						pEntity->TakeDamage(pAttackPlayer->pev, pAttackPlayer->pev, m_SkillDamage, DMG_BULLET | DMG_NEVERGIB);
					}
				}
			}
		}

		void StartFadeOut()
		{
			if (pev->rendermode == kRenderNormal) {
				pev->renderamt = 255.0f;
				pev->rendermode = kRenderTransTexture;
			}

			pev->solid = SOLID_NOT;
			pev->avelocity = g_vecZero;
			pev->nextthink = gpGlobals->time + 0.03s;

			SetThink(&CLanceFountainSpecialAttack::FadeOut);
		}

		void FadeOut()
		{
			if (pev->renderamt > 50) {
				pev->renderamt -= 52.0f;
				pev->nextthink = gpGlobals->time + 0.03s;
			}
			else 
			{
				pev->renderamt = 0.0f;
				SUB_Remove();
			}
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_lance_specialattack1.mdl");
			PRECACHE_MODEL("models/ef_lance_specialattack2.mdl");
			PRECACHE_MODEL("models/ef_lance_specialattack3.mdl");

			PRECACHE_SOUND("weapons/lance_special_attack_inoperation.wav");
		}


		void Init(float flDamage, int iSkill, Vector vecDir)
		{
			m_SkillDamage = flDamage;
			m_vecDir = vecDir;
			m_iSkill = iSkill;
		}

		int m_iDoDamage;
		float m_SkillDamage;
		Vector m_vecDir;
		time_point_t m_tTimeRemove;
		int m_iSkill;
	};

	LINK_ENTITY_TO_CLASS(lance_fountainspecial, CLanceFountainSpecialAttack);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	
#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

	class CKnifeLance : public LinkWeaponTemplate<CKnifeLance,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_lance";
		static constexpr const char* V_Model = "models/v_lance.mdl";
		static constexpr const char* P_Model = "models/p_lance.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = 100;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "dragonsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum lance_e
		{
			ANIM_IDLE,
			ANIM_DRAW,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_SNDATTACK,
			ANIM_FIELD_ATTACK_CHECK,
			ANIM_FIELD_ATTACK_START,
			ANIM_FIELD_ATTACK_CHARGE,
			ANIM_FIELD_ATTACK_ACTUATION,
			ANIM_FIELD_ATTACK_INOPERATION,
			ANIM_FIELD_ATTACK_END,
			ANIM_SPECIAL_ATTACK,
			ANIM_SPECIAL_ATTACK_INOPERATION,
			ANIM_SPECIAL_ATTCK_END
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_lance.mdl");
			PRECACHE_MODEL("models/p_lance.mdl");

			
			PRECACHE_MODEL("sprites/ef_lance_hit.spr");
#ifndef CLIENT_DLL
			PRECACHE_MODEL("sprites/ef_lance_cockpung.spr");
			PRECACHE_MODEL("sprites/muzzleflash159.spr");
#endif
			PRECACHE_SOUND("weapons/lance_slash1.wav");
			PRECACHE_SOUND("weapons/lance_slash2.wav");
			PRECACHE_SOUND("weapons/lance_slash3.wav");
			PRECACHE_SOUND("weapons/lance_slash3_exp.wav");

			PRECACHE_SOUND("weapons/lance_slash_hit1.wav");
			PRECACHE_SOUND("weapons/lance_slash_hit2.wav");
			PRECACHE_SOUND("weapons/lance_slash_hit3.wav");	
			
			PRECACHE_SOUND("weapons/lance_snd_attack.wav");

			PRECACHE_SOUND("weapons/lance_special_attack.wav");
			PRECACHE_SOUND("weapons/lance_special_attack_inoperation.wav");

			PRECACHE_SOUND("weapons/lance_wall_metal1.wav");
			PRECACHE_SOUND("weapons/lance_wall_metal2.wav");
			PRECACHE_SOUND("weapons/lance_wall_stone1.wav");
			PRECACHE_SOUND("weapons/lance_wall_stone2.wav");

			//PRECACHE_SOUND("weapons/lance_field_attack_actuation.wav");	in model
			PRECACHE_SOUND("weapons/lance_field_attack_start.wav");
			PRECACHE_SOUND("weapons/lance_field_attack_charge.wav");
			PRECACHE_SOUND("weapons/lance_field_attack_charge2.wav");	// Where???
			//PRECACHE_SOUND("weapons/lance_field_attack_inoperation_spark.wav");	entity
			PRECACHE_SOUND("weapons/lance_field_attack_inoperation.wav");	
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
			m_iState2 = 0;
			m_iCheck = 0;
			m_iActSlash3 = 0;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;
			m_tTimeRestMaxSpeed = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragonsword", UseDecrement() != FALSE, 0.1s);
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
		void Slash3Think();
		void OnAttack2();
		void OnAttack2Released();
		void ComboAttack();
		void FakeKnockBack();
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iType);
		void MaterialSound();
# endif
		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 30;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 332.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1240.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 70;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 900.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage *= 3600.0f;
#endif
			return flDamage;
		}
		float FieldAttackDamage() const
		{
			float flDamage = 4;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 110.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 240.0f;
#endif
			return flDamage;
		}
		float SpecialAttackDamage() const
		{
			float flDamage = 35;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 850.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1280.0f;
#endif
			return flDamage;
		}
		float Slash3ExpDamage() const
		{
			float flDamage = 95;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1100.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4900.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;
		time_point_t m_tTimeSoundEmit;
		time_point_t m_tTimeRestMaxSpeed;
		int m_iDamageType;
		int m_iState;	//pev_iuser1
		int m_iState2;	//pev_iuser4
		int m_iActSlash3;
		int m_iCheck;	//g_iCheck
		int m_iFrozen;	//g_Frozen
		int m_iCheckSpecial;	//CheckSpecial
	};

	LINK_ENTITY_TO_CLASS(knife_lance, CKnifeLance)

	void CKnifeLance::Spawn()
	{
		pev->classname = MAKE_STRING("knife_lance");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_tTimeRestMaxSpeed = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		m_iState2 = 0;
		m_iCheck = 0;
		m_iActSlash3 = 0;
		FallInit();
	}

	int CKnifeLance::GetItemInfo(ItemInfo* p)
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
	void CKnifeLance::MaterialSound()
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192;
		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
		char chTextureType = 0;
		char szbuffer[64];
		if (tr.flFraction < 1)
		{
			const char* pTextureName;
			pTextureName = TRACE_TEXTURE(tr.pHit, vecSrc, vecEnd);

			if (pTextureName)
			{
				// strip leading '-0' or '+0~' or '{' or '!'
				if (*pTextureName == '-' || *pTextureName == '+')
					pTextureName += 2;

				if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
					pTextureName++;

				// '}}'
				Q_strcpy(szbuffer, pTextureName);
				szbuffer[CBTEXTURENAMEMAX - 1] = '\0';

				// get texture type
				chTextureType = TEXTURETYPE_Find(szbuffer);
			}

			switch (chTextureType)
			{
				case CHAR_TEX_METAL:
				case CHAR_TEX_VENT:
				case CHAR_TEX_COMPUTER:
					{
						if(RANDOM_LONG(0,1))
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_wall_metal1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						else
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_wall_metal2.wav", VOL_NORM, ATTN_NORM, 0, 94);					
						break;
					}
				case CHAR_TEX_WOOD:
				case CHAR_TEX_SLOSH:
					{
						if (RANDOM_LONG(0, 1))
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						else
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						break;
					}
				default:
				{
					if (RANDOM_LONG(0, 1))
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_wall_metal1.wav", VOL_NORM, ATTN_NORM, 0, 94);
					else
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_wall_metal2.wav", VOL_NORM, ATTN_NORM, 0, 94);
					break;
				}
			}
		}
	}

	hit_result_t CKnifeLance::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iType)
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
					if (!iType)
					{
						//left
						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.3f });

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(3.0s);
							strategy->SetStunGravityTime(3.0s);
						}	
					}
					else if(iType == 1)
					{
						//left3
						if (player->m_bIsZombie && player->m_ZombieClass)
						{
							if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
							{
								Vector2D vecDir = (player->pev->origin - pAttackPlayer->pev->origin).Make2D().Normalize();

								player->pev->velocity.x = vecDir.x * 2400.0;
								player->pev->velocity.y = vecDir.y * 2400.0;
							}
						}
					}
					else
					{
						//right
						if (player->m_bIsZombie && player->m_ZombieClass)
						{
							if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
							{
								Vector2D vecDir = (player->pev->origin - pAttackPlayer->pev->origin).Make2D().Normalize();

								player->pev->velocity.x = vecDir.x * 2100.0;
								player->pev->velocity.y = vecDir.y * 2100.0;
								player->pev->velocity.z = 290.0;
							}
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
				result = HIT_WALL;
		}

		return result;
	}
#endif

	void CKnifeLance::DelayPrimaryAttack(void)
	{
		
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 2))
			{
			case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			MaterialSound();
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	void CKnifeLance::PrimaryAttack(void)
	{
		if (m_iCheck == 3)
		{
			m_iCheck = 4;
			m_iCheckSpecial = 1;
			ComboAttack();

			return;
		}

		switch (m_iState)
		{
			case 0:
			case 1:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				
				m_iState = 2;
				SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
				SetThink(&CKnifeLance::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.2s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;				
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;
				if ((pev->button & IN_ATTACK) && !(pev->oldbuttons & IN_ATTACK))
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;
				
				if (!m_iCheck)
					m_iCheck = 1;
				else if (m_iCheck == 3)
					m_iCheck = 3;
				else
					m_iCheck = 1;
				break;
			}
			case 2:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 3;
				SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);

				SetThink(&CKnifeLance::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.33s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;

				if (!m_iCheck)
					m_iCheck = 1;
				else if (m_iCheck == 3)
					m_iCheck = 3;
				else
					m_iCheck = 1;
				break;
			}
			case 3:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_slash3.wav", VOL_NORM, ATTN_NORM, 0, 94);

				SendWeaponAnim(ANIM_SLASH3, UseDecrement() != FALSE);
				SetThink(&CKnifeLance::Slash3Think);
				pev->nextthink = gpGlobals->time + 0.5s;

				m_iState = 0;

				if (!m_iCheck)
					m_iCheck = 1;
				else if (m_iCheck == 3)
					m_iCheck = 3;
				else
					m_iCheck = 1;

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.67s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.04s;
				break;
			}
		}
	}

	void CKnifeLance::FakeKnockBack()
	{
		float flGround = 600.0, flAir = 400.0, flAirStrafing = 400.0, flDucking = 600.0, flVelocityModifier = 1.0;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;
		Vector2D vecKickRateDir = (m_pPlayer->pev->origin.Make2D() - vecEnd.Make2D()).Normalize();

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		{
			if (m_pPlayer->pev->velocity.Length2D() > 0)
			{
				m_pPlayer->pev->velocity.x = vecKickRateDir.x * flAirStrafing * flVelocityModifier;
				m_pPlayer->pev->velocity.y = vecKickRateDir.y * flAirStrafing * flVelocityModifier;
			}
			else
			{
				m_pPlayer->pev->velocity.x = vecKickRateDir.x * flAir * flVelocityModifier;
				m_pPlayer->pev->velocity.y = vecKickRateDir.y * flAir * flVelocityModifier;
			}
		}
		else
		{
			if (FBitSet(pev->flags, FL_DUCKING))
			{
				m_pPlayer->pev->velocity.x = vecKickRateDir.x * flDucking;
				m_pPlayer->pev->velocity.y = vecKickRateDir.y * flDucking;
			}
			else
			{
				m_pPlayer->pev->velocity.x = vecKickRateDir.x * flGround;
				m_pPlayer->pev->velocity.y = vecKickRateDir.y * flGround;
			}
		}
	
	}

	void CKnifeLance::Slash3Think()
	{
		if (!m_iActSlash3)
		{
			
#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
			{
			case HIT_PLAYER:
			{
				switch (RANDOM_LONG(0, 2))
				{
				case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				}			
				break;
			}
			case HIT_WALL:
			{
				MaterialSound();
				break;
			}
			}	
#endif
			m_iActSlash3 = 1;
			SetThink(nullptr);
			SetThink(&CKnifeLance::Slash3Think);
			pev->nextthink = gpGlobals->time + 0.67s;
		}
		else
		{	
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecOrigin = vecSrc + gpGlobals->v_forward * 55;

#ifndef CLIENT_DLL
			KnifeAttack(Slash3ExpDamage(), 260, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 1);
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, vecOrigin, g_vecZero, 0.0, 0.0, 8, 0, FALSE, FALSE);
#endif			
			FakeKnockBack();
			m_iActSlash3 = 0;
			SetThink(nullptr);
		}

		m_iState = 0;
		
	}

	void CKnifeLance::SecondaryAttack(void)
	{
		if (!m_iState2)
		{
			SendWeaponAnim(ANIM_FIELD_ATTACK_CHECK, UseDecrement() != FALSE);

			m_iState2 = 1;

			m_flTimeCheckCharge = gpGlobals->time + 0.2s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
		}
		if (m_flTimeCheckCharge < gpGlobals->time)
		{
			switch (m_iState2)
			{
			case 1:
			{
				SendWeaponAnim(ANIM_FIELD_ATTACK_START, UseDecrement() != FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_field_attack_start.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState2 = 2;

				m_flTimeCheckCharge = gpGlobals->time + 0.66s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.67s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85s;
				break;
			}
			case 2:
			{
				SendWeaponAnim(ANIM_FIELD_ATTACK_CHARGE, UseDecrement() != FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_field_attack_charge.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState2 = 3;

				m_flTimeCheckCharge = gpGlobals->time + 1.13s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.33s;
				break;
			}
			case 3:
			{
				m_tTimeSoundEmit = gpGlobals->time + 0.01s;
				m_flTimeCheckCharge = gpGlobals->time + 999999s;
				m_iState2 = 4; //ChareFinsh
				break;
			}
			}
		}
	}

	void CKnifeLance::ComboAttack()
	{
		switch (m_iCheck)
		{
		case 4:
		{
			SendWeaponAnim(ANIM_SPECIAL_ATTACK, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
			{
			case HIT_PLAYER:
			{
				switch (RANDOM_LONG(0, 2))
				{
				case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				}

				break;
			}
			case HIT_WALL:
			{
				MaterialSound();
				break;
			}
			}
#endif
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.42s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.35s;
			m_pPlayer->m_flNextAttack = 2.4s;
			SetThink(&CKnifeLance::ComboAttack);
			m_iCheck = 5;
			pev->nextthink = gpGlobals->time + 1.16s;

			m_iState = m_iState2 = 0;
			break;
		}
		case 5:
		{
			SendWeaponAnim(ANIM_SPECIAL_ATTACK_INOPERATION, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

			m_tTimeRestMaxSpeed = gpGlobals->time + 0.7s;
			m_pPlayer->pev->velocity.x = 0.0;
			m_pPlayer->pev->velocity.y = 0.0;
			m_pPlayer->ResetMaxSpeed();
			//do buff
#ifndef CLIENT_DLL
			Vector vecAngle = m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle;
			vecAngle[0] = 0;
			UTIL_MakeVectors(vecAngle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecOrigin = vecSrc + gpGlobals->v_forward * 80;
			Vector vecEnd = vecOrigin;
			vecEnd[2] -= 150;	
			TraceResult tr;
			UTIL_TraceHull(vecOrigin, vecEnd, dont_ignore_monsters, point_hull, ENT(m_pPlayer->pev), &tr);
			if (tr.flFraction != 1.0f)
				vecOrigin = tr.vecEndPos;
			else
				vecOrigin[2] -= 64;
			CLanceFountainSpecialAttack* pEnt = CLanceFountainSpecialAttack::Create("lance_fountainspecial", vecOrigin, { 0,0,0 }, ENT(m_pPlayer->pev), 1);
			if (pEnt)
			{
				pEnt->Init(SpecialAttackDamage(), 0, gpGlobals->v_forward);
			}
#endif
			m_iCheck = 6;
			SetThink(nullptr);
			SetThink(&CKnifeLance::ComboAttack);
			pev->nextthink = gpGlobals->time + 0.33s;
			m_iFrozen = 1;

			//m_pPlayer->pev->maxspeed = 1.0;
			break;		
		}
		case 6:
		{
			SendWeaponAnim(ANIM_SPECIAL_ATTCK_END, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iFrozen = 0;
			m_iCheck = 0;

			SetThink(nullptr);
			break;
		}
		}
	}

	void CKnifeLance::OnAttack2Released(void)
	{
		switch (m_iState2)
		{
		case 4:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(ANIM_FIELD_ATTACK_ACTUATION, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_field_attack_actuation.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iState2 = 5;
			m_pPlayer->m_flNextAttack = 1.5s;
			SetThink(&CKnifeLance::OnAttack2Released);
			pev->nextthink = gpGlobals->time + 0.53s;
			break;
		}
		case 5:
		{
			SendWeaponAnim(ANIM_FIELD_ATTACK_INOPERATION, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iState2 = 6;

			m_tTimeRestMaxSpeed = gpGlobals->time + 0.7s;
			m_pPlayer->pev->velocity.x = 0.0;
			m_pPlayer->pev->velocity.y = 0.0;
			m_pPlayer->ResetMaxSpeed();

			//do buff
#ifndef CLIENT_DLL
			Vector vecOrigin = m_pPlayer->pev->origin;
			vecOrigin[2] -= 20.0;

			CLanceFountainFieldAttack* pEnt = static_cast<CLanceFountainFieldAttack*>(CBaseEntity::Create("lance_fountainfield", vecOrigin, { 0,0,0 }, ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				pEnt->Init(FieldAttackDamage(), 0, m_pPlayer);
			}
#endif

			SetThink(nullptr);
			SetThink(&CKnifeLance::OnAttack2Released);
			pev->nextthink = gpGlobals->time + 0.33s;
			break;
		}
		case 6:
		{
			SendWeaponAnim(ANIM_FIELD_ATTACK_END, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
			m_iState2 = 0;
			SetThink(nullptr);
			break;
		}
		}
	}

	void CKnifeLance::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_snd_attack.wav", VOL_NORM, ATTN_NORM, 0, 94);

		switch (KnifeAttack(GetSecondaryAttackDamage(), 185, 360, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 2))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 2))
			{
			case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/lance_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			MaterialSound();
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	void CKnifeLance::OnAttack2(void)
	{
		if (m_iCheck == 1)
			m_iCheck = 2;
		else if (m_iCheck == 2)
			m_iCheck = 3;
		else
			m_iCheck = 0;

		SendWeaponAnim(ANIM_SNDATTACK, UseDecrement() != FALSE);

		SetThink(&CKnifeLance::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.3s;
		
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.2s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

		m_iState2 = 0;
	}

	float CKnifeLance::GetMaxSpeed(void)
	{
		if (m_tTimeRestMaxSpeed > gpGlobals->time)
			return 1;
		else if (m_tTimeRestMaxSpeed == invalid_time_point)
			return 252;
		else
			return 252;
	}

	void CKnifeLance::Holster(int skiplocal)
	{
		SetThink(nullptr);
		//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "", VOL_NORM, ATTN_NORM, 0, 94);
		//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "", VOL_NORM, ATTN_NORM, 0, 94);
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeLance::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		m_iState = 0;

		if (m_iState2 && !(m_pPlayer->pev->button & IN_ATTACK2))
		{
			if (m_iState2 == 4)
				OnAttack2Released();
			else
				OnAttack2();
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iCheck = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		
	}

	void CKnifeLance::ItemPostFrame()
	{
		if (!(m_pPlayer->pev->button & IN_ATTACK) && m_iState == 2)
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;
		}

		if (!(m_pPlayer->pev->button & IN_ATTACK) && m_iState == 3)
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;
		}

		if (m_tTimeRestMaxSpeed != invalid_time_point && m_tTimeRestMaxSpeed < gpGlobals->time)
		{
			m_tTimeRestMaxSpeed = invalid_time_point;
			m_pPlayer->ResetMaxSpeed();
		}

		if (m_iState2 == 4)
		{
			if (m_tTimeSoundEmit != invalid_time_point && m_tTimeSoundEmit <= gpGlobals->time)
			{		
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/lance_field_attack_inoperation_spark.wav", VOL_NORM, ATTN_NORM, 0, 94);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/lance_field_attack_charge.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_tTimeSoundEmit = gpGlobals->time + 4s;
			}
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

}
