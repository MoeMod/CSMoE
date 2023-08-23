#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_sbmine.h"

#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_sbmine, CSbMine)

		void CSbMine::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_SBMINE;
		SET_MODEL(ENT(pev), "models/w_sbmine.mdl");

		m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE * 2;
		m_flStartThrow = invalid_time_point;
		m_flReleaseThrow = invalid_time_point;

		m_flTimeOut = invalid_time_point;


		FallInit();
	}

	void CSbMine::Precache(void)
	{
		PRECACHE_MODEL("models/p_sbmine.mdl");
		PRECACHE_MODEL("models/p_sbmine_2.mdl");
		PRECACHE_MODEL("models/w_sbmine.mdl");

		m_iModelIndex = PRECACHE_MODEL("models/v_sbmine.mdl");
	}

	int CSbMine::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "SbMine";
		p->iMaxAmmo1 = MAX_AMMO_HEGRENADE * 2;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 3;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_SBMINE;
		p->iWeight = HEGRENADE_WEIGHT;
		p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

		return 1;
	}

	BOOL CSbMine::Deploy(void)
	{
		SetThink(nullptr);

		m_flReleaseThrow = invalid_time_point;
		m_fMaxSpeed = 260;

#ifndef CLIENT_DLL
		ItemPostFrame();
#endif

		return DefaultDeploy("models/v_sbmine.mdl", m_pDeployRemoteControl ? "models/p_sbmine_2.mdl" : "models/p_sbmine.mdl", m_pDeployRemoteControl ? SBMINE_TRIGGER_DRAW : SBMINE_DRAW, "grenade", UseDecrement() != FALSE);
	}

	BOOL CSbMine::CanHolster(void)
	{
		return CBasePlayerWeapon::CanHolster();
	}
	void CSbMine::Holster(int skiplocal)
	{
		SetThink(nullptr);

		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_pPlayer->RemoveWeapon(m_iId);
			DestroyItem();
		}

#ifndef CLIENT_DLL
		ItemPostFrame();
#endif

		m_flStartThrow = invalid_time_point;
		m_flReleaseThrow = invalid_time_point;
	}

	void CSbMine::PrimaryAttack(void)
	{

		if (GetTimeOut())
			return;

		if (!m_pDeployRemoteControl)
		{
			if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
			{
				m_flStartThrow = gpGlobals->time;
				m_flReleaseThrow = invalid_time_point;
				SendWeaponAnim(SBMINE_SHOOT_READY, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;

				SetThink(&CSbMine::AnimThink);
				pev->nextthink = gpGlobals->time + 0.86s;
			}
		}

	}
	void EXPORT CSbMine::AnimThink(void)
	{
		pev->nextthink = gpGlobals->time + 2.03s;

		if (m_flStartThrow != invalid_time_point)
			SendWeaponAnim(4, UseDecrement() != FALSE);
	}
	void CSbMine::SecondaryAttack(void)
	{

		if (m_pDeployRemoteControl)
		{
#ifndef CLIENT_DLL
			if (m_pMine)
			{
				SendWeaponAnim(SBMINE_TRIGEER_SHOOT, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;

				SetThink(&CSbMine::DelayThink);
				pev->nextthink = gpGlobals->time + 0.766s;

			}
#endif
		}
	}
	void EXPORT CSbMine::DelayThink(void)
	{
		pev->nextthink = gpGlobals->time;
#ifndef CLIENT_DLL
		if (m_pMine)
		{
			m_pMine->SetIsManualExp(true);
			Holster(false);
			RetireWeapon();

			SetThink(nullptr);
		}
#endif
	}
#ifndef CLIENT_DLL
	void CSbMine::ItemPostFrame(void)
	{
		if (m_flTimeOut != invalid_time_point)
		{
			if (pev->modelindex != m_iModelIndex)
				pev->modelindex = m_iModelIndex;

			if (gpGlobals->time <= m_flTimeOut)
			{

				if (GetBodygroup(3) == 0)
				{
					SetBodygroup(3, 1);
				}
			}
			else //m_flTimeOut <= gpGlobals->time 
			{

				if (GetBodygroup(3) == 1)
				{
					SetBodygroup(3, 0);
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sbmine_beep.wav", VOL_NORM, ATTN_NORM);
				}
				m_flTimeOut = invalid_time_point;
			}
		}
		else
		{
			if (pev->modelindex != m_iModelIndex)
				pev->modelindex = m_iModelIndex;

			if (GetBodygroup(3) == 1)
			{
				SetBodygroup(3, 0);
			}
		}

		//#ifndef CLIENT_DLL
		//		UTIL_ClientPrintAll(HUD_PRINTTALK, "m_flTimeOut:%s  bool = %s GetBodygroup = %s",std::to_string((m_flTimeOut-gpGlobals->time) / 1.0s).c_str(),std::to_string(GetTimeOut()).c_str(), std::to_string(GetBodygroup(3)).c_str());
		//#endif

		return CBasePlayerWeapon::ItemPostFrame();
	}
#endif
	void CSbMine::WeaponIdle(void)
	{
		if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
			m_flReleaseThrow = gpGlobals->time;

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		SetThink(nullptr);

		if (m_flStartThrow != invalid_time_point)
		{

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
			SendWeaponAnim(SBMINE_SHOOT2, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1); //5
#endif
			//Make Bomb...Lazy to do 
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL

			m_pMine = CSbMineNade::Create(vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev));

			if (m_pMine)
			{
				m_pMine->Init(m_pPlayer, this);
				m_pMine->SetIsManualExp(false);
				m_pMine->pev->velocity = gpGlobals->v_forward * 1050;
			}
#endif

			m_flStartThrow = invalid_time_point;

			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_pDeployRemoteControl = true;

			if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;

			return;
		}
		else if (m_flReleaseThrow != invalid_time_point)
		{
			m_flStartThrow = invalid_time_point;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				SendWeaponAnim(m_pDeployRemoteControl ? SBMINE_TRIGGER_DRAW : SBMINE_DRAW, UseDecrement() != FALSE);
				m_flReleaseThrow = invalid_time_point;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
			}
			else
				RetireWeapon();

			return;
		}

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(m_pDeployRemoteControl ? SBMINE_IDLE_TRIGGER : SBMINE_IDLE, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
	}
	BOOL CSbMine::CanDeploy(void)
	{
		return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
	}


#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(effect_sbmine, CSbMineEffect);
	CSbMineEffect* CSbMineEffect::Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iType)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("effect_sbmine"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSbMineEffect* pMineEffect = (CSbMineEffect*)Instance(pent);

		if (pMineEffect)
		{
			if (iType)
				pMineEffect->m_flAliveTime = invalid_time_point;

			pMineEffect->m_iType = iType;

			pMineEffect->pev->owner = pentOwner;
			pMineEffect->pev->origin = vecOrigin;
			pMineEffect->pev->angles = vecAngles;
			if (iType)
			{
				pMineEffect->pev->angles = g_vecZero;
				pMineEffect->pev->angles.y = -vecAngles.y;

			}
			pMineEffect->Spawn();
		}

		return pMineEffect;
	}
	void CSbMineEffect::Init(CBasePlayer* pOwner, CSbMine* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;
	}
	void CSbMineEffect::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), (!m_iType) ? "models/ef_sbmine_explosion.mdl" : "models/ef_sbmine_lightning.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY;
		pev->framerate = 1.0;
		pev->frame = 0;
		pev->sequence = 0;

		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255.0;

		if (m_iType == 1)
		{
			pev->effects |= EF_NOCULL;
			m_flAliveTime = gpGlobals->time + 2.33s;
		}

		pev->classname = MAKE_STRING("effect_sbmine");
		pev->nextthink = gpGlobals->time + 0.01s;
		pev->animtime = gpGlobals->time;

		SetThink(&CSbMineEffect::EffectThink);
	}

	void CSbMineEffect::Precache(void)
	{
		PRECACHE_MODEL("models/ef_sbmine_explosion.mdl");
		PRECACHE_MODEL("models/ef_sbmine_lightning.mdl");
	}

	void EXPORT CSbMineEffect::EffectThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.25s;

		if (!m_pOwner)
		{
			SUB_Remove();
			return;
		}

		if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
		{
			SUB_Remove();
			return;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];

		if (!pWeapon)
		{
			SUB_Remove();
			return;
		}

		if (pWeapon->m_iId != GetWeaponsId())
		{
			SUB_Remove();
			return;
		}

		if (m_iType != 1)
		{
			if (pev->rendermode != kRenderTransTexture)
			{
				pev->renderamt = 255.0f;
				pev->rendermode = kRenderTransTexture;
			}
			if (pev->renderamt > 50)
			{
				pev->renderamt -= 30.0f;
				if (pev->skin < 5)
					pev->skin++;
				else
					pev->skin = 5;

			}
			else
			{
				pev->renderamt = 0.0f;
				pev->nextthink = gpGlobals->time + 0.1s;
				SetThink(&CSbMineEffect::SUB_Remove);
			}
		}
		else
		{
			if (m_flAliveTime != invalid_time_point && m_flAliveTime < gpGlobals->time)
			{
				m_flAliveTime = invalid_time_point;
				SetThink(nullptr);

				pev->nextthink = gpGlobals->time + 0.01s;
				SetThink(&CSbMineEffect::FadeOut);
			}
		}
	}

	void CSbMineEffect::StartFadeOut()
	{
		if (pev->rendermode != kRenderTransTexture)
		{
			pev->renderamt = 255.0f;
			pev->rendermode = kRenderTransTexture;
		}
		pev->nextthink = gpGlobals->time + 0.01s;

		SetThink(&CSbMineEffect::FadeOut);
	}

	void CSbMineEffect::FadeOut()
	{
		if (pev->renderamt > 25)
		{
			pev->renderamt -= 25.0f;
			pev->nextthink = gpGlobals->time + 0.01s;
		}
		else
		{
			pev->renderamt = 0.0f;
			pev->nextthink = gpGlobals->time + 0.01s;
			SetThink(&CSbMineEffect::SUB_Remove);
		}
	}


	LINK_ENTITY_TO_CLASS(mine_sbmine, CSbMineNade);

	CSbMineNade* CSbMineNade::Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("mine_sbmine"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSbMineNade* pMine = (CSbMineNade*)Instance(pent);

		if (pMine)
		{
			pMine->pev->owner = pentOwner;
			pMine->pev->origin = vecOrigin;
			pMine->pev->angles = vecAngles;
			pMine->Spawn();
		}

		return pMine;
	}
	void CSbMineNade::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), "models/w_sbmine.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->angles = g_vecZero;
		pev->gravity = 2.5;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_TOSS;
		pev->frame = 0;

		m_bIsManualExp = false;
		m_flNextAnim = invalid_time_point;
		m_flWarningTime = invalid_time_point;
		m_flDelayExplosion = invalid_time_point;

		pev->classname = MAKE_STRING("mine_sbmine");
		pev->nextthink = gpGlobals->time + 0.01s;
		SetThink(&CSbMineNade::MineThink);
	}

	void CSbMineNade::Init(CBasePlayer* pOwner, CSbMine* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;
	}
	void CSbMineNade::Precache(void)
	{

		m_iExp[0] = PRECACHE_MODEL("sprites/ef_sbmine_explosion.spr");
		//m_iExp[1] = PRECACHE_MODEL("models/ef_sbmine_explosion.mdl");

		//m_iLightning = PRECACHE_MODEL("models/ef_sbmine_lightning.mdl");

		m_iDebuffField = PRECACHE_MODEL("models/ef_sbmine.mdl");
		m_iDebuffIcon = PRECACHE_MODEL("sprites/ef_sbmine_debuff.spr");

		m_iLightReady = PRECACHE_MODEL("sprites/sbmine_light_ready.spr");


		PRECACHE_SOUND("weapons/sbmine_warning.wav");
		PRECACHE_SOUND("weapons/sbmine_timeout.wav");
		PRECACHE_SOUND("weapons/sbmine_drop.wav");
		PRECACHE_SOUND("weapons/sbmine_exp.wav");
		PRECACHE_SOUND("weapons/sbmine_beep.wav");
	}

	void EXPORT CSbMineNade::MineThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		if (!m_pOwner)
		{
			Remove();
			return;
		}

		if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
		{
			Remove();
			return;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];

		if (!pWeapon)
		{
			Remove();
			return;
		}

		if (pWeapon->m_iId != GetWeaponsId())
		{
			Remove();
			return;
		}

		//Do not start detecting before touching the ground.
		if (pev->flags & FL_ONGROUND)
		{
			if (!m_iState)
			{
				//If Already on the ground. Set freezetime to detect.
				m_iState = 1;
				m_flNextAnim = gpGlobals->time + flMineReadyTime;

				ResetSequenceInfo();
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sbmine_drop.wav", VOL_NORM, ATTN_NORM);
			}
			else if (m_iState == 1 && m_flNextAnim != invalid_time_point && m_flNextAnim < gpGlobals->time)
			{
				m_iState = 2;
				m_flNextAnim = invalid_time_point;
			}
			else if (m_iState == 2)
				this->DetectionThink();
			else if (m_iState == 3)
				this->DebuffThink();

		}
	}
	void CSbMineNade::DetectionThink(void)
	{
		pev->nextthink = gpGlobals->time + flMineDetectionRate;

		Vector pOrigin = pev->origin;
		CBaseEntity* iIdDetected = NULL;

		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flMineDetectionRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == m_pOwner)
				continue;

			//队友不触发检测
			if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
				continue;

			if (UTIL_IsWallBetweenEntity(pEntity, this))
				continue;

			iIdDetected = pEntity;
			CreateWarningField(iIdDetected, 1);
		}
		if (iIdDetected)
		{
			if (iIdDetected->IsAlive())
			{
				this->pev->origin.x = iIdDetected->pev->origin.x;
				this->pev->origin.y = iIdDetected->pev->origin.y;

				m_iState = 3;
				m_flWarningTime = gpGlobals->time + flMineDebuffTime;

				CreateWarningField(nullptr, 0);
				return;
			}
		}
		else
		{
			if (m_bIsManualExp)
			{
				m_iState = 3;
				m_flWarningTime = gpGlobals->time + (m_bIsManualExp ? flMineDebuffTimeManual : flMineDebuffTime);

				CreateWarningField(nullptr, 0);
				return;
			}
		}
	}
	void CSbMineNade::DebuffThink(void)
	{
		pev->nextthink = gpGlobals->time + flMineDebuffRate;

		if (m_flWarningTime != invalid_time_point)
		{
			if (m_flWarningTime >= gpGlobals->time)
			{
				RadiusDamage(GetDamageDebuff(0), flMineExplosionRadius, true, false, false);
			}
			else
			{
				m_iState = 4;
				m_flWarningTime = invalid_time_point;

				Explode(false);
			}
		}
	}
	void CSbMineNade::ExplodeEffect(Vector vecOrigin)
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z + 60.0f);
		WRITE_SHORT(m_iExp[0]);
		WRITE_BYTE(16);
		WRITE_BYTE(255);
		WRITE_BYTE(20);
		MESSAGE_END();

#ifndef CLIENT_DLL

		CSbMineEffect* m_pSbMineEffect = CSbMineEffect::Create(vecOrigin, pev->angles, ENT(m_pOwner->pev));

		if (m_pSbMineEffect)
			m_pSbMineEffect->Init(m_pOwner, m_pWeapon);

		CSbMineEffect* m_pSbMineEffectLightning = CSbMineEffect::Create(vecOrigin, pev->angles, ENT(m_pOwner->pev), 1);

		if (m_pSbMineEffectLightning)
			m_pSbMineEffectLightning->Init(m_pOwner, m_pWeapon);
#endif

		RadiusDamage(GetDamage(), flMineExplosionRadius, false, true, true, vecOrigin);
	}
	void EXPORT CSbMineNade::OnExplosion(void)
	{

		m_iState = 6;
		m_flDelayExplosion = invalid_time_point;

		//MinePenetration
		Vector vecOrigin; Vector vecOriginCopy; Vector vecEnd;
		vecOriginCopy = vecOrigin = pev->origin;
		vecOriginCopy.z += 8192;

		Vector vecDir = Vector(0, 0, -8192);

		TraceResult tr;
		if ((UTIL_PointContents(vecOrigin) != CONTENTS_WATER))
		{
			for (int i = 0; i < iMinePenetration; i++)
			{
				//慢慢试探实体下方是不是地面 如果不是就慢慢-z坐标
				for (int j = 0; j < 1024; j++)
				{
					vecOriginCopy.z -= 4.0;
					if ((UTIL_PointContents(vecOriginCopy) != CONTENTS_SOLID))
						break;
				};
				vecEnd = vecDir + vecOriginCopy;

				UTIL_TraceLine(vecOriginCopy, vecEnd, ignore_monsters, ENT(0), &tr);

				vecOriginCopy = tr.vecEndPos;

				if (tr.flFraction == 1.0)
					break;

				if ((UTIL_PointContents(vecOriginCopy) == CONTENTS_SOLID))
					break;

				if (!vecOriginCopy.Length())
					vecOriginCopy.z = 1.0;

				Vector vecNewOrigin;
				vecNewOrigin = pev->origin;

				vecNewOrigin.z = vecOriginCopy.z;

				ExplodeEffect(vecNewOrigin);
			}
		}

		ExplodeEffect(vecOrigin);
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/sbmine_exp.wav", VOL_NORM, ATTN_NORM);

		pev->effects |= EF_NODRAW; // 0x80u

		SetThink(nullptr);
		SetThink(&CSbMineNade::LightningThink);
		pev->nextthink = gpGlobals->time + 0.05s;
	}

	void EXPORT CSbMineNade::LightningThink(void)
	{
		pev->nextthink = gpGlobals->time + flMineDebuffRate;
		if (m_iState == 6)
		{
			static int iLightningCount = 0;
			if (iLightningCount < iMineLightningCount)
			{
				//RadiusDamage3(this->pev->origin, this->pev, m_pOwner->pev, GetDamageDebuff(1), flMineLightningRadius, CLASS_NONE, DMG_EXPLOSION, false, false, false, 0);
				RadiusDamage(GetDamageDebuff(1), flMineLightningRadius, false, false, false);
				++iLightningCount;
			}
			else
			{
				SetThink(nullptr);
				pev->nextthink = gpGlobals->time;

				Remove();
			}
		}
	}

	void CSbMineNade::CreateWarningField(CBaseEntity* pEntity, int iType)
	{
		if (iType)
		{
			if (!pEntity)
				return;
			if (!pEntity->IsPlayer())
				return;
			CBasePlayer* pVictim = dynamic_cast<CBasePlayer*>(pEntity);

			if (pVictim->IsAlive())
			{
				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pVictim->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pVictim->entindex());
				WRITE_BYTE(23);
				MESSAGE_END();
			}
		}
		else
		{
			UTIL_TempModel(g_vecZero, g_vecZero, g_vecZero, m_iDebuffField, 60, 0, 15, 0, 0, 255, kRenderTransAdd, this, false, 0, 0, 256, 0);
			EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/sbmine_warning.wav", VOL_NORM, ATTN_NORM);

			if (m_pWeapon)
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];
				if (pWeapon->m_iId == WEAPON_SBMINE)
				{

					CSbMine* pLinkWeapon = dynamic_cast<CSbMine*>(pWeapon);
					if (pLinkWeapon)
					{
						pLinkWeapon->m_pDeployRemoteControl = false;
						pLinkWeapon->SetTimeOut(5.0s);
						pLinkWeapon->Holster(false);
						pLinkWeapon->RetireWeapon();
					}
				}
			}
		}

	}
	void CSbMineNade::Explode(bool IsManual)
	{
		m_iState = 5;

		if (m_flDelayExplosion == invalid_time_point)
		{
			SetThink(&CSbMineNade::OnExplosion);
			pev->nextthink = gpGlobals->time + 0.2s;
		}
	}
	void CSbMineNade::Remove()
	{
		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		SetThink(nullptr);
		return UTIL_Remove(this);
	}

	void CSbMineNade::RadiusDamage(float flDamage, float flRadius, bool IsAttract, bool IsDistanceModified, bool IsSuper, Vector vecOrigin)
	{
		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		bool IsUsingDefaultOrigin = !vecOrigin.Length() ? true : false;

		if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
		{
			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecDelta;
			Vector vecEnd;
			int bInWater = (UTIL_PointContents(IsUsingDefaultOrigin ? pev->origin : vecOrigin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, IsUsingDefaultOrigin ? pev->origin : vecOrigin, flRadius)) != NULL)
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

				//队友不触发检测
				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (!player)
						continue;

					if (pAttackPlayer->m_iTeam == player->m_iTeam)
						continue;

					vecEnd = player->BodyTarget(IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					UTIL_TraceLine(IsUsingDefaultOrigin ? pev->origin : vecOrigin, vecEnd, ignore_monsters, pev->owner, &tr);

					if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
						continue;

					vecDelta = pEntity->pev->origin - (IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					if (!IsAttract)
					{
						if (IsSuper)
							player->pev->velocity.z = 300.0;
					}
					else
					{
						player->pev->velocity = g_vecZero;
						player->m_tLockBuffTime = gpGlobals->time + flMineDebuffRate;
						player->m_flLockGravity = 50.0f;
						player->m_flLockSpeed = 50.0f;
						player->ResetMaxSpeed();
					}

					if (vecDelta.Length() > flRadius)
						continue;

					if (IsDistanceModified)
					{
						if (vecDelta.Length() > flRadius / 2)
							flDamage /= 2;
					}

					player->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION | DMG_NEVERGIB | DMG_NOMORALE);
				}
				else
				{
					vecEnd = pEntity->BodyTarget(IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					UTIL_TraceLine(IsUsingDefaultOrigin ? pev->origin : vecOrigin, vecEnd, ignore_monsters, pev->owner, &tr);

					if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
						continue;

					vecDelta = pEntity->pev->origin - (IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					if (vecDelta.Length() > flRadius)
						continue;

					if (IsDistanceModified)
					{
						if (vecDelta.Length() > flRadius / 2)
							flDamage /= 2;
					}

					pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION | DMG_NEVERGIB | DMG_NOMORALE);
				}
			}
		}
	}

	float CSbMineNade::GetDamage() const
	{
		float flDamage = 1.0f;

		flDamage = 100.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 16900.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 39400.0f;
		return flDamage;
	}
	float CSbMineNade::GetDamageDebuff(int iStage) const
	{
		float flDamage = 1.0f;

		flDamage = iStage ? 10.0f : 5.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = iStage ? 500.0f : 450.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = iStage ? 2000.0f : 1500.0f;
		return flDamage;
	}

#endif
}


