
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "cbase/cbase_hash.h"
#include "player.h"
#include "entity_state.h"

#include "gamemode/zb1/zb1_const.h"
#include "gamemode/zbz/zbz_const.h"
#include "gamemode/zbz/zbz_skill.h"
#include "weapons/KnifeAttack.h"

namespace sv {
	int g_iKnifeMasterSpr;
	int g_iExplodeSpr;
	void Skill_Precache()
	{
		g_iKnifeMasterSpr = PRECACHE_MODEL("sprites/ef_coilmg.spr");
		g_iExplodeSpr = PRECACHE_MODEL("sprites/zerogxplode.spr");
		PRECACHE_MODEL("models/zombiez_contactinfection_ring.mdl");
		PRECACHE_SOUND("zombi/ghosthunterready.wav");		
	}

	class ZSkill_BulletDamage : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_BulletDamage(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "破甲弹头"; }
		const char* GetDesc() const override { return "增加武器的射击威力。"; } // 射击武器追加（10+1*回合数）的伤害
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (!m_pPlayer->m_pActiveItem)
				return;
			if (m_pPlayer->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
				return;
			if (~bitsDamageType & DMG_BULLET)
				return;
			
			flDamage += 10 + 1 * g_pGameRules->m_iTotalRoundsPlayed;
		}
	};

	class ZSkill_Critical : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Critical(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "致命一击"; }
		const char* GetDesc() const override { return "低概率触发触发致命一击。"; } // 有（4%+0.5%*回合数）的概率追加（100%+2%*回合数）的伤害
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (!m_pPlayer->m_pActiveItem)
				return;
			if (m_pPlayer->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
				return;
			if (RANDOM_FLOAT(0, 1) > 0.04f + 0.005f * g_pGameRules->m_iTotalRoundsPlayed)
				return;

			flDamage *= 1.0f + 0.02f * g_pGameRules->m_iTotalRoundsPlayed;
		}
	};

	class ZSkill_KnifeMaster : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_KnifeMaster(CBasePlayer* player) : CZombieZSkill_Base(player), m_flNextThink(0s) {}
		const char* GetName() const override { return "格斗大师"; };
		const char* GetDesc() const override { return "近战武器加强。"; }; // 近身武器攻击时获得一个（80units60°）的攻击，伤害为（120+40*回合数），击退为（600+30*回合数），定身为（20%+3%*回合数），最大定身50%加成
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }

		void OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int& iClip, int& iBackAmmo)
		{
			if (!IsActivate())
				return;
			if (weapon->m_iId != WEAPON_KNIFE)
				return;
			if (m_flNextThink > gpGlobals->time)
				return;
			m_flNextThink = gpGlobals->time + 0.3s;

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			auto result = KnifeAttack3(vecSrc, gpGlobals->v_forward, 120.0f + 40.0f * g_pGameRules->m_iTotalRoundsPlayed, 80, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);

			/*if (result == HIT_PLAYER) {
				ApplyKnockbackData(victim, victim->pev->origin - m_pPlayer->pev->origin, { 700, 1600, 1300, 400, 0.5f });
			}*/
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecEnd = vecSrc + gpGlobals->v_forward * 80;
			TraceResult tr;
			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(tr.vecEndPos.x);
			WRITE_COORD(tr.vecEndPos.y);
			WRITE_COORD(tr.vecEndPos.z);
			WRITE_SHORT(g_iKnifeMasterSpr);
			WRITE_BYTE(3);
			WRITE_BYTE(15);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();
		}

	//	void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
	//		if (attacker != m_pPlayer)
	//			return;
	//		if (!IsActivate())
	//			return;
	//		if (m_pPlayer->m_pActiveItem->iItemSlot() != KNIFE_SLOT)
	//			return;
	//		if (m_flNextThink > gpGlobals->time)
	//			return;
	//		m_flNextThink = gpGlobals->time + 0.01s;

	//		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	//		Vector vecSrc = m_pPlayer->GetGunPosition();
	//		auto result = KnifeAttack3(vecSrc, gpGlobals->v_forward, 120.0f + 40.0f * g_pGameRules->m_iTotalRoundsPlayed, 80, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
	//		if (result == HIT_NONE)
	//			return;
	//		/*if (result == HIT_PLAYER) {
	//			ApplyKnockbackData(victim, victim->pev->origin - m_pPlayer->pev->origin, { 700, 1600, 1300, 400, 0.5f });
	//		}*/
	//		Vector vecEnd = vecSrc + gpGlobals->v_forward * 80;
	//		TraceResult tr;
	//		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	//		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	//		WRITE_BYTE(TE_EXPLOSION);
	//		WRITE_COORD(tr.vecEndPos.x);
	//		WRITE_COORD(tr.vecEndPos.y);
	//		WRITE_COORD(tr.vecEndPos.z);
	//		WRITE_SHORT(g_iKnifeMasterSpr);
	//		WRITE_BYTE(3);
	//		WRITE_BYTE(15);
	//		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
	//		MESSAGE_END();
	//	}
	private:
		time_point_t m_flNextThink;
	};

	class ZSkill_DoubleJump : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_DoubleJump(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "二段跳"; }
		const char* GetDesc() const override { return "在空中可以再跳一次。"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnGetSkill() override {
			SET_PHYSICS_KEY_VALUE(m_pPlayer->edict(), "dbj", "1");
		}
		void OnBecomeHuman() override {
			OnGetSkill();
		}
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}
	};

	class ZSkill_Health : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Health(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "生命补液"; }
		const char* GetDesc() const override { return "增加生命值。"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnBecomeHuman() override {
			OnGetSkill();
		}
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}

		void OnEvolution(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}

		void OnGetSkill() override {
			
			m_pPlayer->pev->max_health = m_pPlayer->pev->health += m_pPlayer->m_bIsZombie ? GetZombieBonus() : GetBonus();
		}
	private:
		float GetBonus() const { return 400.0f + 200.0f * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetZombieBonus() const { return 1000.0f + 500.0f * g_pGameRules->m_iTotalRoundsPlayed; }
	};

	class ZSkill_BombBackpack : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_BombBackpack(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "炸弹背包"; }
		const char* GetDesc() const override { return "一定周期自动生成手雷，每（30秒-0.5秒*回合数）生成（1+1*回合数/6）个高爆手雷"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnBecomeHuman() override {
			m_flNextThink = gpGlobals->time + GetBonus();
		}
		void OnThink() override {

			if (m_pPlayer->m_iBuff & BUFF_GHOSTHUNTER)
				return;

			if (m_flNextThink > gpGlobals->time)
				return;

			m_flNextThink = gpGlobals->time + GetBonus();
			if (!IsActivate())
				return;

			int i = m_pPlayer->GetAmmoIndex("HEGrenade");
			if (i < 0)
				return;

			int amount = m_pPlayer->AmmoInventory(i);
			if (amount >= GetAmountBonus())
				return;

			if (amount) {
				m_pPlayer->m_rgAmmo[i]++;
				// make sure the ammo messages have been linked first
				if (gmsgAmmoPickup)
				{
					// Send the message that ammo has been picked up
					MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
					WRITE_BYTE(i); // ammo ID
					WRITE_BYTE(1); // amount
					MESSAGE_END();
				}
				return;
			}
			m_pPlayer->GiveNamedItem("weapon_hegrenade");
		}
		void OnGetSkill() override {
			m_flNextThink = time_point_t();
		}
	private:
		time_point_t m_flNextThink;
		duration_t GetBonus() const { return 30.0s - 0.5s * g_pGameRules->m_iTotalRoundsPlayed; }
		int GetAmountBonus() const { return 1 + g_pGameRules->m_iTotalRoundsPlayed / 6; }
	};

	class ZSkill_Kangaroo : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Kangaroo(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "袋鼠"; }
		const char* GetDesc() const override { return "增加跳跃力（30+5*回合数）"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnGetSkill() override {
			/*const char* pExtraPhysics = g_engfuncs.pfnGetPhysicsKeyValue(m_pPlayer->edict(), "ext");
			int flags = UTIL_ReadFlags(pExtraPhysics);
			flags |= (1 << 1);
			char szFlags[32];
			UTIL_GetFlags(szFlags, flags);*/
			char szValue[16];
			Q_sprintf(szValue, "%d", GetBonus());
			SET_PHYSICS_KEY_VALUE(m_pPlayer->edict(), "kgr", szValue);
		}

		/*void OnRoundStart() override {
			OnGetSkill();
		}*/
		void OnBecomeHuman() override {
			OnGetSkill();
		}
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}
	private:
		int GetBonus() const { return 30 + 5 * g_pGameRules->m_iTotalRoundsPlayed; }
	};

	class ZSkill_ExplodeBullet : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ExplodeBullet(CBasePlayer* player) : CZombieZSkill_Base(player), m_flNextThink(0s), m_uFiredTimes(0){}
		const char* GetName() const override { return "爆炸弹头"; }
		const char* GetDesc() const override { return "射击时一定概率触发爆炸。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnTraceAttack(CBasePlayer* victim, CBasePlayer* attacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (m_pPlayer->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
				return;
			if(m_flNextThink > gpGlobals->time)
				return;
			m_flNextThink = gpGlobals->time + 0.01s;
			if (++m_uFiredTimes < GetRequiredTimes())
				return;
			if (UTIL_PointContents(ptr->vecEndPos) == CONTENTS_WATER)
				return;

			m_uFiredTimes = 0;
			RadiusDamage3(ptr->vecEndPos, m_pPlayer->pev, m_pPlayer->pev, flDamage * (0.35f + 0.05f * g_pGameRules->m_iTotalRoundsPlayed), 64.0f, CLASS_MACHINE, DMG_EXPLOSION, 0, 0, 0, 0);

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(ptr->vecEndPos.x);
			WRITE_COORD(ptr->vecEndPos.y);
			WRITE_COORD(ptr->vecEndPos.z + 30.0f);
			WRITE_SHORT(g_iExplodeSpr);
			WRITE_BYTE(RANDOM_LONG(3, 6));
			WRITE_BYTE(50);
			WRITE_BYTE(TE_EXPLFLAG_NONE);
			MESSAGE_END();
		}
	private:
		time_point_t m_flNextThink;
		unsigned int GetRequiredTimes() const { return max(10, 30 - 4 * g_pGameRules->m_iTotalRoundsPlayed); }
		unsigned int m_uFiredTimes;
	};

	class ZSkill_Speed : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Speed(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "猎豹"; }
		const char* GetDesc() const override { return "增加移动速度。"; } // （8+2*回合数）
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }

		void OnResetMaxSpeed() override {
			m_pPlayer->pev->maxspeed += GetBonus();
		}
		void OnGetSkill() override {
			m_pPlayer->ResetMaxSpeed();
		}
	private:
		float GetBonus() const { return min(8.0f + 2.0f * g_pGameRules->m_iTotalRoundsPlayed, 300.0f); }
	};

	class ZSkill_Icarus : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Icarus(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "伊卡洛斯"; }
		const char* GetDesc() const override { return "持续按下跳跃键，短时间内可以滑翔。跳跃过程中碰撞到其他物体时无法启动。"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }

		void OnTouch(CBaseEntity* other) override
		{
			if (m_pPlayer->pev->flags & FL_ONGROUND)
				return;

			if (m_pPlayer->pev->groundentity && m_pPlayer->pev->groundentity == other->edict())
				return;

			m_sSkillStatus = 2;
		}

		void OnButtonEvent(unsigned short& bitsCurButton, int bitsOldButton) override {

			if (bitsCurButton & ~bitsOldButton & IN_JUMP)
			{
				m_sSkillStatus = 0;
			}

			if (~bitsCurButton & IN_JUMP)
				return;

			if (m_pPlayer->pev->flags & FL_ONGROUND)
			{
				if (m_sSkillStatus <= 1)
				{
					m_sSkillStatus = 0;
					m_flAirTime = invalid_time_point;
				}
				return;
			}

			if (!m_sSkillStatus && m_flAirTime == invalid_time_point)
			{
				m_sSkillStatus = 1;
				m_flAirTime = gpGlobals->time;
			}

			if (m_sSkillStatus != 1 || m_flCooldown > gpGlobals->time)
				return;

			if (m_flAirTime + 0.5s <= gpGlobals->time)
			{
				Vector vecFwd, vecRight, vecUp;
				Vector vecAngles;
				vecAngles = m_pPlayer->pev->v_angle;
				vecAngles[0] = 0.0f;
				UTIL_MakeVectorsPrivate(vecAngles, vecFwd, vecRight, vecUp);
				Vector vecOrigin = m_pPlayer->GetGunPosition() + (vecFwd * 200) + vecRight + (vecUp * 100);
				float speed = 300.0f;
				Vector vecVelocity = vecOrigin - m_pPlayer->pev->origin;
				float num = sqrtf(speed * speed / (vecVelocity.x * vecVelocity.x + vecVelocity.y * vecVelocity.y + vecVelocity.z * vecVelocity.z));
				vecVelocity.x *= (num * 2.0);
				vecVelocity.y *= (num * 2.0);
				vecVelocity.z *= (num / 2.0);
				m_pPlayer->pev->velocity = vecVelocity;
				m_sSkillStatus = 2;
				m_flCooldown = gpGlobals->time + 2s;

				MESSAGE_BEGIN(MSG_PVS, gmsgZBZMsg, m_pPlayer->pev->origin);
				WRITE_BYTE(ZBZ_MESSAGE_WING); // type, reserved.
				WRITE_BYTE(m_pPlayer->entindex());
				MESSAGE_END();
			}
		}

		void OnGetSkill() override {
			m_sSkillStatus = 0;
			m_flAirTime = invalid_time_point;
		}

	private:
		short m_sSkillStatus;
		time_point_t m_flAirTime;
		time_point_t m_flCooldown;
	};

	class ZSkill_Fanatic : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Fanatic(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "狂热"; }
		const char* GetDesc() const override { return "通过战斗与合作变得更加强大。如果是人类，将激活士气系统。如果是僵尸，将激活怒气系统"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnGetSkill() override {
			MESSAGE_BEGIN(MSG_ONE, gmsgZB3Msg, nullptr, m_pPlayer->edict());
			WRITE_BYTE(ZB3_MESSAGE_MORALE); // type, reserved.
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgZB3Msg, nullptr, m_pPlayer->edict());
			WRITE_BYTE(ZB3_MESSAGE_RAGE); // type, reserved.
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			MESSAGE_END();
		}
	};

	class ZSkill_HeroicPresence : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_HeroicPresence(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "英雄出现"; }
		const char* GetDesc() const override { return "拥有概率成为英雄"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
	};

	class ZSkill_SteelSkin : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SteelSkin(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "倾斜盔甲"; }
		const char* GetDesc() const override { return "受到攻击时，一定概率不受到伤害"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (victim != m_pPlayer)
				return;
			if (!IsActivate())
				return;

			if (m_flTime > gpGlobals->time)
				flDamage = 0.0f;

			if (RANDOM_FLOAT(0, 1) > 0.01f + 0.005f * g_pGameRules->m_iTotalRoundsPlayed)
				return;

			flDamage = 0.0f;
			m_flTime = gpGlobals->time + 0.01s;
		}
	private:
		time_point_t m_flTime;
	};

	class ZSkill_EnduranceHave : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_EnduranceHave(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "坚持不懈"; }
		const char* GetDesc() const override { return "受到攻击时低概率忽略定身和击退"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnApplyKnockback(CBasePlayer* attacker, KnockbackData& data) override {
			if (!IsActivate())
				return;

			if (m_flTime > gpGlobals->time)
			{
				data.flOnGround = data.flFlying = data.flDucking = data.flNotOnGround = -1.0f;
				data.flVelocityModifier = 0.0f;
			}
				

			if (RANDOM_FLOAT(0, 1) > 0.01f + 0.005f * g_pGameRules->m_iTotalRoundsPlayed)
				return;

			data.flOnGround = data.flFlying = data.flDucking = data.flNotOnGround = -1.0f;
			data.flVelocityModifier = 0.0f;
			m_flTime = gpGlobals->time + 0.01s;
		}
	private:
		time_point_t m_flTime;
	};

	class ZSkill_ArmonedAdvance : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ArmonedAdvance(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "猛犸"; }
		const char* GetDesc() const override { return "缓慢移动时增加防御力。"; } // 缓慢移动0.1秒后降低伤害、击退、定身
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnThink() override {
			float speed = m_pPlayer->pev->velocity.Length2D();
			if (speed > 165.0f)
			{
				m_Status = 0;
				return;
			}

			if (!m_Status)
			{
				m_Status = 1;
				m_flTime = gpGlobals->time + 0.1s;
				return;
			}

			if (m_Status != 1 || m_flTime > gpGlobals->time)
			{
				return;
			}

			m_Status = 2;
		}

		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (victim != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (m_Status != 2)
				return;

			flDamage *= GetBonus();
		}

		void OnApplyKnockback(CBasePlayer* attacker, KnockbackData& data) override {
			if (!IsActivate())
				return;
			if (m_Status != 2)
				return;

			data.flFlying = data.flDucking = data.flNotOnGround *= GetKnockbackBonus();
			data.flVelocityModifier *= GetVelModifiedBonus();
		}
	private:
		float GetBonus() const { return max(0.0f, 1.0f - (0.1f + 0.02f * g_pGameRules->m_iTotalRoundsPlayed)); }
		float GetKnockbackBonus() const { return max(0.0f, 1.0f - (0.1f + 0.03f * g_pGameRules->m_iTotalRoundsPlayed)); }
		float GetVelModifiedBonus() const { return max(0.0f, 1.0f - (0.1f + 0.4f * g_pGameRules->m_iTotalRoundsPlayed)); }
		time_point_t m_flTime;
		byte m_Status;
	};

	class ZSkill_SpeedyCrouch : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SpeedyCrouch(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "脚掌"; }
		const char* GetDesc() const override { return "蹲下移动是增加速度。"; } // （5+2*回合数）
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnThink() override {
			if (!IsActivate())
				return;

			if (m_pPlayer->pev->flags & FL_ONGROUND && m_pPlayer->pev->flags & FL_DUCKING)
			{
				if (!m_bActivate) {
					m_bActivate = true;
					m_pPlayer->ResetMaxSpeed();
				}
			}
			else if(m_bActivate) {
				m_bActivate = false;
				m_pPlayer->ResetMaxSpeed();
			}
		}

		void OnResetMaxSpeed() override {
			if(m_bActivate)
				m_pPlayer->pev->maxspeed += GetBonus();
		}
	private:
		float GetBonus() const { return 50.0f + 5.0f * g_pGameRules->m_iTotalRoundsPlayed; }
		bool m_bActivate;
	};

	class ZSkill_SteelBody : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SteelBody(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "钢铁铠甲"; }
		const char* GetDesc() const override { return "增加护甲"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}
		void OnGetSkill() override {
			m_pPlayer->pev->armorvalue *= GetBonus();
		}
	private:
		float GetBonus() const { return 1.0f + (0.2f + 0.05f * g_pGameRules->m_iTotalRoundsPlayed); }
	};

	class ZSkill_Specialist : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Specialist(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "持之以恒"; }
		const char* GetDesc() const override { return "解锁人类特殊技能"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnGetSkill() override {
			//show hud
		}
	};

	class ZSkill_StealthReloader : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_StealthReloader(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "换弹透明"; }
		const char* GetDesc() const override { return "换弹时身体变得透明，如果进行攻击，透明状态立即解除"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t& fDelay) override
		{
			if (m_flCooldown > gpGlobals->time)
				return;

			m_flTime = gpGlobals->time + GetBonus();
			m_flCooldown = gpGlobals->time + GetCooldown();
		}

		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;

			m_flTime = invalid_time_point;
		}

		void OnAddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override
		{
			if (!player)
				return;
			if (m_flTime <= gpGlobals->time)
				return;

			state->rendermode = kRenderTransAlpha;
			state->renderamt = 1;
		}
	private:
		duration_t GetBonus() const { return 0.5s + 0.025s * g_pGameRules->m_iTotalRoundsPlayed; }
		duration_t GetCooldown() const { return 1.25s + 0.025s * g_pGameRules->m_iTotalRoundsPlayed; }
		time_point_t m_flCooldown;
		time_point_t m_flTime;
	};

	class ZSkill_RapidReloader : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_RapidReloader(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "高速填装"; }
		const char* GetDesc() const override { return "加快武器的填装速度"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t& fDelay) override
		{
			fDelay *= GetBonus();
		}

	private:
		float GetBonus() const { return max(0.0f, 1.0f - (0.32f + 0.022f * g_pGameRules->m_iTotalRoundsPlayed)); }
	};

	class ZSkill_AmmoReserves : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_AmmoReserves(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "备弹补充"; }
		const char* GetDesc() const override { return "增加武器的备弹"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax, int& iMax) override
		{
			iMax += GetBonus();
		}
	private:
		int GetBonus() const { return min(999, 50 + 10 * g_pGameRules->m_iTotalRoundsPlayed); }
	};

	class ZSkill_DropZone : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_DropZone(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "强制坠落"; }
		const char* GetDesc() const override { return "攻击在梯子上的敌人时，强制敌人摔落"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (victim->pev->movetype != MOVETYPE_FLY)
				return;

			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(victim->m_pModStrategy.get());
			if (strategy) {
				strategy->SetLadderDisabled(true);
				SET_PHYSICS_KEY_VALUE(victim->edict(), "drp", "1");
			}
		}
	};
	
	class ZSkill_ExplosiveReaction : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ExplosiveReaction(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "爆弹兽颅"; }
		const char* GetDesc() const override { return "受到一定伤害后，会获得爆弹兽颅"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnSpawn() override {
			m_flTakeDamage = 0.0f;
		}
		void OnGetSkill() override {
			m_flTakeDamage = 0.0f;
		}
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (victim != m_pPlayer)
				return;
			if (!IsActivate())
				return;

			m_flTakeDamage += flDamage;
			if (m_flTakeDamage >= GetBonus())
			{
				m_flTakeDamage = 0.0f;

				int i = m_pPlayer->GetAmmoIndex("HEGrenade");
				if (i < 0)
					return;

				if (m_pPlayer->AmmoInventory(i)) {
					m_pPlayer->m_rgAmmo[i]++;
					// make sure the ammo messages have been linked first
					if (gmsgAmmoPickup)
					{
						// Send the message that ammo has been picked up
						MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
						WRITE_BYTE(i); // ammo ID
						WRITE_BYTE(1); // amount
						MESSAGE_END();
					}
					return;
				}

				victim->GiveNamedItem("weapon_zombibomb");
			}
		}
	private:
		float m_flTakeDamage;
		float GetBonus() const { return max(0.0f, 4000.0f - (50.0f * g_pGameRules->m_iTotalRoundsPlayed)); }
	};

	class ZSkill_BombEnhacement : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_BombEnhacement(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "兽颅强化"; }
		const char* GetDesc() const override { return "增加爆弹兽颅的攻击力和击退效果。"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnZombiBombExploded(float& flKickRate, float& flRadius, float& flDamage) override {
			flKickRate *= GetKickRateBonus();
			flRadius *= GetRadiusBonus();
			flDamage *= GetDamageBonus();
		}
	private:
		float GetKickRateBonus() const { return 1.0f + 0.02f * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetRadiusBonus() const { return 1.0f + 0.1f * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetDamageBonus() const { return 1.0f + 0.2f * g_pGameRules->m_iTotalRoundsPlayed; }
	};

	class ZSkill_ClawEnhacement : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ClawEnhacement(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "合金利爪"; }
		const char* GetDesc() const override { return "增加攻击距离"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnAdjustZombieKnifeRange(bool bStab, float& flRange) override {
			if (!IsActivate())
				return;

			flRange += 15.0f;

			MESSAGE_BEGIN(MSG_PVS, gmsgZBZMsg, m_pPlayer->pev->origin);
			WRITE_BYTE(ZBZ_MESSAGE_CLAW); // type, reserved.
			WRITE_BYTE(m_pPlayer->entindex());
			MESSAGE_END();
		}
	};

	class ZSkill_IncendiaryAmmo : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_IncendiaryAmmo(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "点燃"; }
		const char* GetDesc() const override { return "使用狙击枪攻击的敌人时，有几率使其被点燃"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (~bitsDamageType & DMG_BULLET)
				return;
			if (!IsActivate())
				return;
			if (!victim->m_bIsZombie)
				return;
			CBasePlayerItem* weapon = m_pPlayer->m_pActiveItem;
			if (weapon == NULL)
				return;
			if (!isSniperRifle(weapon))
				return;
			if (victim->m_LastHitGroup == HITGROUP_HEAD)
			{
				if (RANDOM_FLOAT(0, 1) > GetHeadShotChance())
					return;
			}
			else if (RANDOM_FLOAT(0, 1) > GetChance())
				return;

			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(victim->m_pModStrategy.get());
			if (strategy) {
				strategy->SetOnFire(m_pPlayer, GetFireTime());
			}
		}
	private:
		float GetChance() const { return 0.2f + 0.05f * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetHeadShotChance() const { return 1.0f; }
		duration_t GetFireTime() const { return 1.5s + 0.25s * g_pGameRules->m_iTotalRoundsPlayed; }
	};

	class ZSkill_Marksman : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Marksman(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "神枪手"; }
		const char* GetDesc() const override { return "大幅提升狙击步枪的命中率。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnAdjustFireBullets3Dir(Vector& vecDir) override {
			if (!IsActivate())
				return;
			if (!m_pPlayer->m_pActiveItem)
				return;
			if (!isSniperRifle(m_pPlayer->m_pActiveItem))
				return;

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecNewDir = gpGlobals->v_forward;
			vecDir.x = vecNewDir.x;
			vecDir.y = vecNewDir.y;
			vecDir.z = vecNewDir.z;
		}
	};

	class ZSkill_SteelHead : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SteelHead(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "钢铁头盔"; }
		const char* GetDesc() const override { return "降低致命打击的伤害。"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (victim != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (m_pPlayer->m_LastHitGroup != HITGROUP_HEAD)
				return;

			flDamage += flDamage * (-0.04f + -0.03f * g_pGameRules->m_iTotalRoundsPlayed);
		}
	};

	class ZSkill_AmmoCreation : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_AmmoCreation(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "弹夹扩充"; }
		const char* GetDesc() const override { return "填装时一定概率填装更多的子弹。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnAdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int& iClip) override
		{
			if (!IsActivate())
				return;
			if (RANDOM_FLOAT(0, 1) > 0.5f + 0.03f * g_pGameRules->m_iTotalRoundsPlayed)
				return;

			iClip += iClip * 0.45f + 0.05f * g_pGameRules->m_iTotalRoundsPlayed;
		}
	};

	class ZSkill_ContactInfection : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ContactInfection(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "接触感染"; }
		const char* GetDesc() const override { return "身体接触时触发感染。"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnTouch(CBaseEntity *other) override
		{
			if (!IsActivate())
				return;
			if (!other->IsPlayer())
				return;
			if (!other->IsAlive())
				return;
			CBasePlayer* p = static_cast<CBasePlayer*>(other);
			if (p->m_bIsZombie)
				return;

			p->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, 1000.0f, DMG_BULLET | DMG_NEVERGIB | DMG_INFECTION);
		}

		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}

		void OnGetSkill() override {
			if (!IsActivate())
				return;

			MESSAGE_BEGIN(MSG_ALL, gmsgZBZMsg, nullptr, m_pPlayer->edict());
			WRITE_BYTE(ZBZ_MESSAGE_CONTACT_INFECTION_RING); // type, reserved.
			WRITE_BYTE(m_pPlayer->entindex());
			MESSAGE_END();
		}
	};

	class ZSkill_GeneBooster : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_GeneBooster(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "强化基因"; }
		const char* GetDesc() const override { return "被感染时强化为母体僵尸。"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}

		void OnGetSkill() override {
			if (m_pPlayer->m_iZombieLevel < ZOMBIE_LEVEL_ORIGIN)
			{			
				auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
				if (strategy)
				{
					strategy->SetRagePercent(101.0f);
					strategy->CheckEvolution();
				}
				m_pPlayer->m_iZombieLevel = ZOMBIE_LEVEL_ORIGIN;
			}

		}
	};

	class CZBZFireBall : public CBaseAnimating
	{
	public:
		void Precache()
		{
			PRECACHE_MODEL("models/ef_fireball.mdl");
			m_sModelIndex = PRECACHE_MODEL("sprites/bexplo.spr");
		}
		void Spawn()
		{
			Precache();
			pev->movetype = MOVETYPE_TOSS;

			if (pev->classname)
			{
				RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
			}

			MAKE_STRING_CLASS("zbz_fireball", pev);
			AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
			pev->solid = SOLID_BBOX;

			SetThink(&CZBZFireBall::AnimationThink);
			SetTouch(&CZBZFireBall::Touch);
			SET_MODEL(ENT(pev), "models/ef_fireball.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			pev->sequence = 0;
			pev->frame = 0;
			pev->framerate = 1.5f;
			ResetSequenceInfo();
			m_fSequenceLoops = true;
		}
		static void ShootFireBall(entvars_t* owner, Vector origin, Vector velocity)
		{
			CZBZFireBall* pGrenade = CreateClassPtr<CZBZFireBall>();
			pGrenade->Spawn();
			UTIL_SetOrigin(pGrenade->pev, origin);
			pGrenade->pev->velocity = velocity;
			pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
			pGrenade->pev->owner = ENT(owner);
			pGrenade->pev->renderfx = kRenderFxNone;
			pGrenade->pev->renderamt = 255.0f;
			pGrenade->pev->rendermode = kRenderTransAdd;
			pGrenade->pev->nextthink = gpGlobals->time;
		}

		void EXPORT AnimationThink(void)
		{
			StudioFrameAdvance();
			pev->nextthink = gpGlobals->time + 0.2s;

			if (m_flFireThink > gpGlobals->time)
				return;

			m_flFireThink = gpGlobals->time + 0.5s;
			// TODO:Follow Fire Sprite
		}

		void EXPORT Touch(CBaseEntity* other)
		{
			RadiusDamage3(pev->origin, pev, VARS(pev->owner), 800.0f + 100.0f * g_pGameRules->m_iTotalRoundsPlayed, 100.0f, CLASS_MACHINE, DMG_EXPLOSION | DMG_NOMORALE, 0, 0, 0, 0);

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_sModelIndex);
			WRITE_BYTE(RANDOM_LONG(25, 30));
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NONE);
			MESSAGE_END();

			UTIL_Remove(this);
			SetTouch(NULL);
		}
	private:
		short m_sModelIndex;
		time_point_t m_flFireThink;
	};
	LINK_ENTITY_TO_CLASS(zbz_fireball, CZBZFireBall);

	class ZSkill_Fireball : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Fireball(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "火焰球"; }
		const char* GetDesc() const override { return "使用狙击枪射击时发射火焰球。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnAdjustFireBullets3Dir(Vector& vecDir) override {
			if (!IsActivate())
				return;
			if (!m_pPlayer->m_pActiveItem)
				return;
			if (!isSniperRifle(m_pPlayer->m_pActiveItem))
				return;
			if (m_flCooldown > gpGlobals->time)
				return;

			m_flCooldown = gpGlobals->time + GetCooldown();
			CZBZFireBall::ShootFireBall(m_pPlayer->pev, m_pPlayer->GetGunPosition(), vecDir * GetSpeed());
		}
	private:
		float GetDamage() const { return 800.0f + 100.0f * g_pGameRules->m_iTotalRoundsPlayed; }
		duration_t GetCooldown() const { return 5.0s - 0.2s * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetKnockBack() const { return 320.0f + 20.0f * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetSpeed() const { return 1200.0f; }
		float GetRange() const { return 100.0f; }
		time_point_t m_flCooldown;
	};

	class ZSkill_SixthSense : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SixthSense(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "第六感"; }
		const char* GetDesc() const override { return "可以感知母体僵尸的出现。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnThink() override {
			if (!IsActivate())
				return;

			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy && strategy->IsPickAsZombie())
				return;

			if (m_flNextThink > gpGlobals->time)
				return;

			m_flNextThink = gpGlobals->time + 0.5s;
			
			moe::range::PlayersList list;
			std::vector<CBasePlayer*> players{ list.begin(), list.end() };
			players.erase(std::remove_if(players.begin(), players.end(), [=](CBasePlayer* player) {
				auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(player->m_pModStrategy.get());
				if (strategy && strategy->IsPickAsZombie() && player->IsAlive() && (player->pev->origin - m_pPlayer->pev->origin).IsLengthLessThan(250))
				{
					return false;
				}
				return true;
			}), players.end());

			if (players.empty())
				return;

			char szHudText[64];
			Q_sprintf(szHudText, "感知到周围存在生化病毒感染体！");

			hudtextparms_t textParms;
			textParms.x = -1.0f;
			textParms.y = 0.3f;
			textParms.effect = 0;
			textParms.r1 = 200;
			textParms.g1 = 200;
			textParms.b1 = 50;
			textParms.r2 = 255;
			textParms.g2 = 255;
			textParms.b2 = 255;
			textParms.fadeinTime = 0.1f;
			textParms.fadeoutTime = 0.1f;
			textParms.holdTime = 0.5f;
			textParms.fxTime = 0.1f;
			textParms.channel = 1;
			UTIL_HudMessage(m_pPlayer, textParms, szHudText);
		}
	private:
		time_point_t m_flNextThink;
	};

	class ZSkill_Intellectual : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Intellectual(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "进化学者"; }
		const char* GetDesc() const override { return "通过击败僵尸及感染人类也可以提升进化等级。"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }

		void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override
		{
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (!strategy)
				return;
			if (m_pPlayer->m_bIsZombie)
				return;
			if (m_pPlayer->pev != pKiller)
				return;

			strategy->AddExp(max(1, 150 - g_pGameRules->m_iTotalRoundsPlayed * 5));
		}

		void OnInfection(CBasePlayer* victim, CBasePlayer* attacker) override
		{
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (!strategy)
				return;
			if (m_pPlayer != attacker)
				return;

			strategy->AddExp(max(1, 250 - g_pGameRules->m_iTotalRoundsPlayed * 8));
		}
	};

	class ZSkill_LifePlunder : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_LifePlunder(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "感染恢复"; }
		const char* GetDesc() const override { return "感染时恢复周围僵尸的生命值。"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnInfection(CBasePlayer* victim, CBasePlayer* attacker) override
		{
			if (m_pPlayer != attacker)
				return;
			
			for (int i = 1; i <= gpGlobals->maxClients; i++)
			{
				CBasePlayer* player = (CBasePlayer*)UTIL_PlayerByIndex(i);

				if (player == NULL)
					continue;

				if (FNullEnt(player->pev))
					continue;

				if (player->IsDormant())
					continue;

				if (player->m_iTeam != m_pPlayer->m_iTeam)
					continue;

				if (player->pev->health >= player->pev->max_health)
					continue;

				if ((player->pev->origin - m_pPlayer->pev->origin).IsLengthGreaterThan(100.0f))
					continue;

				player->pev->health = min(player->pev->health + player->pev->max_health * (0.03f * g_pGameRules->m_iTotalRoundsPlayed), player->pev->max_health);
				
				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, player->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(player->entindex());
				WRITE_BYTE(2);
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, player->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(player->entindex());
				WRITE_BYTE(3);
				MESSAGE_END();
			}
		}
	};
	
	class ZSkill_RapidFireAmmo : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_RapidFireAmmo(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "连发手雷"; }
		const char* GetDesc() const override { return "使用投掷装备时，自动追加使用次数。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }

		struct ShootGrenadeRecord
		{
			time_point_t flTime;
			int iCount;
			Vector vecStart;
			Vector vecVelocity;
			duration_t time;
			int iTeam;
			unsigned short usEvent;
		};

		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			m_vecGrenadeRecord.clear();
		}

		void OnSpawn() override {
			m_vecGrenadeRecord.clear();
		}

		void OnShootGrenade(Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent) override {
			if (!IsActivate())
				return;

			m_flNextThink = time_point_t();
			ShootGrenadeRecord record;
			record.flTime = gpGlobals->time + 1s;
			record.iCount = 1 + g_pGameRules->m_iTotalRoundsPlayed / 2;
			record.vecStart = vecStart;
			record.vecVelocity = vecVelocity;
			record.time = time;
			record.iTeam = iTeam;
			record.usEvent = usEvent;
			m_vecGrenadeRecord.push_back(record);
			//m_vecGrenadeRecord.push_back({ gpGlobals->time, 1 + g_pGameRules->m_iTotalRoundsPlayed / 2, vecStart, vecVelocity, time, iTeam, usEvent });
		}

		void OnThink() override {
			if (!IsActivate())
				return;

			if (m_flNextThink > gpGlobals->time)
				return;

			m_flNextThink = gpGlobals->time + 1s;

			if (m_vecGrenadeRecord.empty())
				return;

			auto iter = m_vecGrenadeRecord.begin();
			while (iter != m_vecGrenadeRecord.end())
			{
				if (iter->flTime > gpGlobals->time)
				{
					++iter;
					continue;
				}

				if (iter->iCount > 0)
				{
					CGrenade* pGrenade = CreateClassPtr<CGrenade>();
					pGrenade->Spawn();

					UTIL_SetOrigin(pGrenade->pev, iter->vecStart);
					pGrenade->pev->velocity = iter->vecVelocity;
					pGrenade->pev->angles = m_pPlayer->pev->angles;
					pGrenade->pev->owner = ENT(m_pPlayer->pev);

					pGrenade->m_usEvent = iter->usEvent;

					pGrenade->SetTouch(&CGrenade::BounceTouch);

					pGrenade->pev->dmgtime = gpGlobals->time + iter->time;
					pGrenade->SetThink(&CGrenade::TumbleThink);
					pGrenade->pev->nextthink = gpGlobals->time + 0.1s;

					pGrenade->pev->sequence = RANDOM_LONG(3, 6);
					pGrenade->pev->framerate = 1.0f;

					pGrenade->m_bJustBlew = true;

					pGrenade->pev->gravity = 0.55f;
					pGrenade->pev->friction = 0.7f;

					pGrenade->m_iTeam = iter->iTeam;

					SET_MODEL(ENT(pGrenade->pev), "models/w_hegrenade.mdl");
					pGrenade->pev->dmg = 100.0f;

					(*iter).flTime = gpGlobals->time + 1s;
					(*iter).iCount--;
				}

				if (!iter->iCount) iter = m_vecGrenadeRecord.erase(iter);
				else ++iter;
			}

		}
	private:
		time_point_t m_flNextThink;
		std::vector<ShootGrenadeRecord> m_vecGrenadeRecord;
	};
	
	class ZSkill_Revenge : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Revenge(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "复仇投掷"; }
		const char* GetDesc() const override { return "作为僵尸死亡时，向前方投掷爆弹兽颅。"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override {
			if (!IsActivate())
				return;

			Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

			if (angThrow.x < 0)
				angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
			else
				angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

			float flVel = (90 - angThrow.x) * 6;

			if (flVel > 750)
				flVel = 750;

			UTIL_MakeVectors(angThrow);
			Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
			Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
			CGrenade::ShootZombieBomb(m_pPlayer->pev, vecSrc, vecThrow, 1.5s, m_pPlayer->m_iTeam, NULL);
		}
	};
	
	class CZBZSupportBombardment : public CBaseAnimating
	{
	public:
		void Precache()
		{
			m_sModelIndex_Missle = PRECACHE_MODEL("models/bunkerbuster_missile.mdl");
			PRECACHE_MODEL("models/bunkerbuster_target.mdl");
			PRECACHE_SOUND("weapons/bunkerbuster_fire.wav");
			PRECACHE_SOUND("weapons/bunkerbuster_explosion_1st.wav");
			PRECACHE_SOUND("weapons/bunkerbuster_whistling1.wav");
			PRECACHE_SOUND("weapons/bunkerbuster_whistling2.wav");
			PRECACHE_SOUND("weapons/bunkerbuster_whistling3.wav");

			m_sSpriteIndex_Fire = PRECACHE_MODEL("sprites/bunkerbuster_fire.spr");
			m_sSpriteIndex_Smoke = PRECACHE_MODEL("sprites/bunkerbuster_smoke.spr");
			m_sSpriteIndex_Exp = PRECACHE_MODEL("sprites/bunkerbuster_explosion.spr");
		}

		void Spawn()
		{
			Precache();
			pev->movetype = MOVETYPE_TOSS;

			if (pev->classname)
			{
				RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
			}

			MAKE_STRING_CLASS("zbz_supportbombardment", pev);
			AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
			pev->solid = SOLID_NOT;

			SetThink(&CZBZSupportBombardment::AnimationThink);
			SET_MODEL(ENT(pev), "models/bunkerbuster_target.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			pev->sequence = 0;
			pev->frame = 0;
			pev->framerate = 1.0f;
			ResetSequenceInfo();
			m_fSequenceLoops = true;
		}

		static void CreateSupportBombardment(entvars_t* owner, Vector origin)
		{
			CZBZSupportBombardment* pEntity = CreateClassPtr<CZBZSupportBombardment>();
			pEntity->Spawn();
			UTIL_SetOrigin(pEntity->pev, origin);
			pEntity->pev->gravity = 2.0f;
			pEntity->pev->owner = ENT(owner);
			pEntity->pev->renderfx = kRenderFxNone;
			pEntity->pev->renderamt = 255.0f;
			pEntity->pev->rendermode = kRenderTransAdd;
			pEntity->pev->nextthink = gpGlobals->time;
			pEntity->m_flFireThink = gpGlobals->time + 4.7s;
			pEntity->m_iState = 0;
		}

		void EXPORT AnimationThink(void)
		{
			if(!m_iState)
				StudioFrameAdvance();

			CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pEntityOwner = nullptr;
			if (pOwner && pOwner->IsPlayer())
			{
				pEntityOwner = static_cast<CBasePlayer*>(pOwner);
				if (pEntityOwner && pEntityOwner->m_bIsZombie)
					UTIL_Remove(this);
			}

			pev->nextthink = gpGlobals->time + 0.2s;

			if (m_iState == 2)
			{
				if (m_flHurtThink <= gpGlobals->time)
				{
					m_flHurtThink = gpGlobals->time + 0.5s;
					
					CBaseEntity* pEntity = NULL;
					while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 150)) != NULL)
					{
						if (pEntity->pev->takedamage == DAMAGE_NO)
							continue;

						CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
						if (pOwner && pOwner->IsPlayer())
						{
							pEntityOwner = static_cast<CBasePlayer*>(pOwner);
							if (g_pGameRules->PlayerRelationship(pEntityOwner, pEntity) == GR_TEAMMATE)
								continue;
						}

						pEntity->TakeDamage(pev, VARS(pev->owner), 30.0f, DMG_NEVERGIB | DMG_BURN | DMG_NOMORALE | DMG_EXPLOSION);
					}
				}

				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_FIREFIELD);
				WRITE_COORD(pev->origin[0]);
				WRITE_COORD(pev->origin[1]);
				WRITE_COORD(pev->origin[2] + RANDOM_FLOAT(25.0f, 45.0f));
				WRITE_SHORT(150); // Radius
				WRITE_SHORT(m_sSpriteIndex_Fire);
				WRITE_BYTE(10); // count
				WRITE_BYTE(TEFIRE_FLAG_PLANAR|TEFIRE_FLAG_ADDITIVE);
				WRITE_BYTE(2); // duration
				MESSAGE_END();
			}

			if (m_flFireThink > gpGlobals->time)
				return;

			if (!m_iState)
			{
				TraceResult result;
				Vector ceiling = pev->origin;
				ceiling.z += 8192.0;
				UTIL_TraceLine(pev->origin, ceiling, ignore_monsters, dont_ignore_glass, ENT(pev), &result);

				Vector vecTravel = result.vecEndPos - pev->origin;
				float flSpeed = 2000.0f;
				pev->effects |= EF_NODRAW;
				m_iState = 1;
				duration_t travelTime = vecTravel.Length() / flSpeed * 1s;
				m_flFireThink = gpGlobals->time + travelTime;

				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_PROJECTILE);
				WRITE_COORD(result.vecEndPos[0]);
				WRITE_COORD(result.vecEndPos[1]);
				WRITE_COORD(result.vecEndPos[2]);
				WRITE_COORD(0);
				WRITE_COORD(0);
				WRITE_COORD(-flSpeed);
				WRITE_SHORT(m_sModelIndex_Missle);
				WRITE_BYTE(12); // (life in 1's)
				WRITE_BYTE(ENTINDEX(pev->owner)); // (owner)
				MESSAGE_END();
			}
			else if (m_iState == 1)
			{
				m_iState = 2;
				m_flFireThink = gpGlobals->time + 12s;

				EMIT_SOUND(ENT(pev), CHAN_STATIC, "weapons/bunkerbuster_explosion_1st.wav", VOL_NORM, ATTN_NORM);

				RadiusDamage3(pev->origin, pev, VARS(pev->owner), 500.0f * (5.0f * g_pGameRules->m_iTotalRoundsPlayed), 250.0f, CLASS_MACHINE, DMG_NEVERGIB | DMG_EXPLOSION | DMG_NOMORALE, 0, 0, 0, 0);

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + 230.0f);
				WRITE_SHORT(m_sSpriteIndex_Exp);
				WRITE_BYTE(22);
				WRITE_BYTE(20);
				WRITE_BYTE(TE_EXPLFLAG_NONE);
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_SPRITE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z + 80.0f);
				WRITE_SHORT(m_sSpriteIndex_Smoke);
				WRITE_BYTE(RANDOM_LONG(4, 9));
				WRITE_BYTE(255);
				MESSAGE_END();
			}
			else
			{
				UTIL_Remove(this);
			}
		}

	private:
		short m_sModelIndex_Missle;
		short m_sSpriteIndex_Fire;
		short m_sSpriteIndex_Smoke;
		short m_sSpriteIndex_Exp;
		int m_iState;
		time_point_t m_flHurtThink;
		time_point_t m_flFireThink;
	};
	LINK_ENTITY_TO_CLASS(zbz_supportbombardment, CZBZSupportBombardment);

	class ZSkill_SupportBombardment : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SupportBombardment(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "支援轰炸"; }
		const char* GetDesc() const override { return "手雷爆炸时低概率进行轰炸。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnGrenadeExploded(Vector vecOrigin) override {
			if (!IsActivate())
				return;
			if (RANDOM_FLOAT(0.0f, 1.0f) > 0.01f + 0.01f * g_pGameRules->m_iTotalRoundsPlayed)
				return;

			CZBZSupportBombardment::CreateSupportBombardment(m_pPlayer->pev, vecOrigin);
		}
	};

	class ZSkill_RapidReloaderII : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_RapidReloaderII(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "高速装填II"; }
		const char* GetDesc() const override { return "武器直接消耗备弹"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		
		void OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int &iClip, int &iBackAmmo)
		{
			if (iClip < 0)
				return;

			if (iBackAmmo <= 0)
				return;

			int iClipDecrease = iClipBeforeAttack - iClip;
			if (iBackAmmo - iClipDecrease >= 0)
			{
				iBackAmmo -= iClipDecrease;
				iClip += iClipDecrease;
			}
			else
			{
				iClip += iClipDecrease;
				iClip -= (iClipDecrease - iBackAmmo);
				if (iClip < 0)
					iClip = 0;
				iBackAmmo = 0;
			}
		}
	};

	class ZSkill_ZombieS : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ZombieS(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "生化超越"; }
		const char* GetDesc() const override { return "受到一定伤害可随机进化成特殊僵尸"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnSpawn() override {
			m_flTakeDamage = 0.0f;

			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy) {
				strategy->SetClassChangeTime(0, 0);
			}
		}
		void OnGetSkill() override {
			m_flTakeDamage = 0.0f;

			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy) {
				strategy->SetClassChangeTime(0, 0);
			}
		}
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (victim != m_pPlayer)
				return;
			if (!IsActivate())
				return;

			m_flTakeDamage += flDamage;
			if (m_flTakeDamage >= GetBonus())
			{
				m_flTakeDamage = 0.0f;

				auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(victim->m_pModStrategy.get());
				if (strategy) {

					int ChangeTime = strategy->GetClassChangeTime(0);
					if (ChangeTime == -1)
						return;

					strategy->SetClassChangeTime(0, 1);

					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "按E成为特殊僵尸");
				}
			}
		}
	private:
		float m_flTakeDamage;
		float GetBonus() const { return max(0.0f, 10000.0f - (200.0f * g_pGameRules->m_iTotalRoundsPlayed)); }
	};

	class ZSkill_GhostHunter : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_GhostHunter(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "幽灵猎手"; }
		const char* GetDesc() const override { return "在人类人数小于6时可以变身为幽灵猎手"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnSpawn() override {
			m_flNextThink = invalid_time_point;
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy) {
				strategy->SetClassChangeTime(1, 0);
			}
		}
		void OnGetSkill() override {
			m_flNextThink = invalid_time_point;
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy) {
				strategy->SetClassChangeTime(1, 0);
			}
		}
		void OnThink() override {
			if (m_flNextThink > gpGlobals->time)
				return;

			if (m_pPlayer->m_bIsZombie)
				return;

			int iTeamCT = CountLiveCTPlayers();
			if (iTeamCT <= 6)
			{
				auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
				if (strategy) {

					int ChangeTime = strategy->GetClassChangeTime(1);
					if (ChangeTime == -1)
						return;

					EMIT_SOUND(m_pPlayer->edict(), CHAN_AUTO, "zombi/ghosthunterready.wav", VOL_NORM, ATTN_NORM);

					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "按E成为幽灵猎手");

					strategy->SetClassChangeTime(1, 1);

					m_flNextThink = gpGlobals->time + 9999s;
				}
			}
			
		}
		int CountLiveCTPlayers()
		{
			CBaseEntity* pPlayer = NULL;
			int i = 0;

			while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL) {
				if (FNullEnt(pPlayer->edict()))
					break;

				if (pPlayer->pev->flags & FL_DORMANT)
					continue;

				if (!pPlayer->IsAlive())
					continue;

				if (GetClassPtr<CBasePlayer>(pPlayer->pev)->m_iTeam == CT)
					++i;
			}

			return i;
		}

		
	private:
		float m_flTakeDamage;
		time_point_t m_flNextThink;
	};

	class ZSkill_Adaptability : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Adaptability(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "适应力"; }
		const char* GetDesc() const override { return "随时改变僵尸种类"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }

		void OnSpawn() override {
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy) {
				strategy->SetClassChangeTime(2, 0);
			}
		}
		void OnGetSkill() override {
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
			if (strategy) {
				strategy->SetClassChangeTime(2, 1);
			}
		}

		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}

	};

	class ZSkill_ZombiBombGiveaway : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_ZombiBombGiveaway(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "兽颅增殖"; }
		const char* GetDesc() const override { return "每隔一段时间自动生成爆弹兽颅，红色兽颅生成周期:（30秒-0.5秒*回合数),绿色兽颅生成周期:（25秒-0.5秒*回合数）"; }
		ZombieZSkillType GetType() const override { return TYPE_ZOMBIE; }
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			m_flNextRedBombGive = gpGlobals->time + GetRedBombBonus();
			m_flNextGreenBombGive = gpGlobals->time + GetGreenBombBonus();
		}
		void OnThink() override {

			if (m_flNextRedBombGive < gpGlobals->time)
			{
				m_flNextRedBombGive = gpGlobals->time + GetRedBombBonus();
				if (!IsActivate())
					return;

				int i = m_pPlayer->GetAmmoIndex("HEGrenade");
				if (i < 0)
					return;

				if (m_pPlayer->AmmoInventory(i)) {
					m_pPlayer->m_rgAmmo[i]++;
					// make sure the ammo messages have been linked first
					if (gmsgAmmoPickup)
					{
						// Send the message that ammo has been picked up
						MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
						WRITE_BYTE(i); // ammo ID
						WRITE_BYTE(1); // amount
						MESSAGE_END();
					}
					return;
				}

				m_pPlayer->GiveNamedItem("weapon_zombibomb");
			}

			if (m_flNextGreenBombGive < gpGlobals->time)
			{
				m_flNextGreenBombGive = gpGlobals->time + GetGreenBombBonus();
				if (!IsActivate())
					return;

				int i = m_pPlayer->GetAmmoIndex("SmokeGrenade");
				if (i < 0)
					return;

				if (m_pPlayer->AmmoInventory(i)) {
					m_pPlayer->m_rgAmmo[i]++;
					// make sure the ammo messages have been linked first
					if (gmsgAmmoPickup)
					{
						// Send the message that ammo has been picked up
						MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
						WRITE_BYTE(i); // ammo ID
						WRITE_BYTE(1); // amount
						MESSAGE_END();
					}
					return;
				}

				m_pPlayer->GiveNamedItem("weapon_zombibomb2");
			}	
		}
		void OnGetSkill() override {
			m_flNextRedBombGive = time_point_t();
			m_flNextGreenBombGive = time_point_t();
		}
	private:
		time_point_t m_flNextRedBombGive;
		time_point_t m_flNextGreenBombGive;
		duration_t GetRedBombBonus() const { return 25.0s - 0.5s * g_pGameRules->m_iTotalRoundsPlayed; }
		duration_t GetGreenBombBonus() const { return 30.0s - 0.5s * g_pGameRules->m_iTotalRoundsPlayed; }
	};

	class ZSkill_BombDefense : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_BombDefense(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "兽颅护盾"; }
		const char* GetDesc() const override { return "降低兽颅造成的伤害及击退效果。能够对兽颅起到防御作用，降低兽颅造成的伤害及击退效果。该防御可对所有类型兽颅产生效果。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }

		void OnZombiBombExploded(float& flKickRate, float& flRadius, float& flDamage) override {

			if (!m_pPlayer->m_bIsZombie)
			{
				flKickRate *= GetKickRateBonus();
				flDamage *= GetDamageBonus();
			}		
		}
	private:
		float GetKickRateBonus() const { return 1.0f - 0.05f * g_pGameRules->m_iTotalRoundsPlayed; }
		float GetDamageBonus() const { return 1.0f - 0.05f * g_pGameRules->m_iTotalRoundsPlayed; }
	};

	class ZSkill_SwordMaster : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_SwordMaster(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "刀剑将领"; }
		const char* GetDesc() const override { return "激活致命打击对近身武器的效果。"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
	};

	class ZSkill_Knife2X : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Knife2X(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "屠戮之刃"; }
		const char* GetDesc() const override { return "开启后近战武器伤害*2"; }
		ZombieZSkillType GetType() const override { return TYPE_HUMAN; }
		void OnGetSkill() override {
			//show hud
		}
	};

	template<class T>
	std::shared_ptr<CZombieZSkill_Base> MakeSkill(CBasePlayer* player)
	{
		return std::make_shared<T>(player);
	}
	const std::tuple<byte, unsigned int, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)> g_SkillList[] = {
		{ Skill_BulletDamage, 100, MakeSkill<ZSkill_BulletDamage> },
		{ Skill_Critical, 100, MakeSkill<ZSkill_Critical> },
		{ Skill_KnifeMaster, 100, MakeSkill<ZSkill_KnifeMaster> },
		{ Skill_DoubleJump, 25, MakeSkill<ZSkill_DoubleJump> },
		{ Skill_Health, 100, MakeSkill<ZSkill_Health> },
		{ Skill_BombBackpack, 100, MakeSkill<ZSkill_BombBackpack> },
		{ Skill_ExplodeBullet, 100, MakeSkill<ZSkill_ExplodeBullet> },
		{ Skill_Speed, 100, MakeSkill<ZSkill_Speed> },
		{ Skill_Icarus, 25, MakeSkill<ZSkill_Icarus> },
		{ Skill_Fanatic, 100, MakeSkill<ZSkill_Fanatic> },

		{ Skill_HeroicPresence, 25, MakeSkill<ZSkill_HeroicPresence> },
		{ Skill_SteelSkin, 100, MakeSkill<ZSkill_SteelSkin> },
		{ Skill_EnduranceHave, 25, MakeSkill<ZSkill_EnduranceHave> },
		{ Skill_ArmonedAdvance, 25, MakeSkill<ZSkill_ArmonedAdvance> },
		{ Skill_SpeedyCrouch, 100, MakeSkill<ZSkill_SpeedyCrouch> },
		{ Skill_SteelBody, 100, MakeSkill<ZSkill_SteelBody> },
		{ Skill_Specialist, 100, MakeSkill<ZSkill_Specialist> },
		{ Skill_StealthReloader, 100, MakeSkill<ZSkill_StealthReloader> },
		{ Skill_RapidReloader, 25, MakeSkill<ZSkill_RapidReloader> },
		{ Skill_AmmoReserves, 25, MakeSkill<ZSkill_AmmoReserves> },

		{ Skill_DropZone, 100, MakeSkill<ZSkill_DropZone> },
		{ Skill_ExplosiveReaction, 50, MakeSkill<ZSkill_ExplosiveReaction> },
		{ Skill_Kangaroo, 25, MakeSkill<ZSkill_Kangaroo> },
		{ Skill_BombEnhacement, 15, MakeSkill<ZSkill_BombEnhacement> },
		{ Skill_ClawEnhacement, 25, MakeSkill<ZSkill_ClawEnhacement> },
		{ Skill_IncendiaryAmmo, 100, MakeSkill<ZSkill_IncendiaryAmmo> },
		{ Skill_Marksman, 100, MakeSkill<ZSkill_Marksman> },
		{ Skill_SteelHead, 100, MakeSkill<ZSkill_SteelHead> },
		{ Skill_AmmoCreation, 100, MakeSkill<ZSkill_AmmoCreation> },
		{ Skill_ContactInfection, 5, MakeSkill<ZSkill_ContactInfection> },

		{ Skill_GeneBooster, 100, MakeSkill<ZSkill_GeneBooster> },
		{ Skill_Fireball, 100, MakeSkill<ZSkill_Fireball> },
		{ Skill_SixthSense, 200, MakeSkill<ZSkill_SixthSense> },
		{ Skill_Intellectual, 100, MakeSkill<ZSkill_Intellectual> },
		{ Skill_LifePlunder, 100, MakeSkill<ZSkill_LifePlunder> },
		{ Skill_RapidFireAmmo, 100, MakeSkill<ZSkill_RapidFireAmmo> },
		{ Skill_Revenge, 100, MakeSkill<ZSkill_Revenge> },
		{ Skill_SupportBombardment, 100, MakeSkill<ZSkill_SupportBombardment> },
		{ Skill_RapidReloaderII, 15, MakeSkill<ZSkill_RapidReloaderII> },
		{ Skill_ZombieS, 50, MakeSkill<ZSkill_ZombieS>},
		{ Skill_GhostHunter, 50, MakeSkill<ZSkill_GhostHunter> },
		{ Skill_Adaptability, 100, MakeSkill<ZSkill_Adaptability> },
		{ Skill_ZombiBombGiveaway, 50, MakeSkill<ZSkill_ZombiBombGiveaway> },
		{ Skill_BombDefense, 100, MakeSkill<ZSkill_BombDefense> },
		{ Skill_SwordMaster, 100, MakeSkill<ZSkill_SwordMaster> },
		{ Skill_Specialist, 100, MakeSkill<ZSkill_Specialist> },
		{ Skill_Knife2X, 100, MakeSkill<ZSkill_Knife2X> },
	};

	constexpr auto NUM_SKILLS = std::extent<decltype(g_SkillList)>::value;

	template <size_t ...N>
	constexpr std::array<std::tuple<byte, unsigned int, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)>, NUM_SKILLS> MakeSkillList(std::index_sequence<N...>) {
		return { g_SkillList[N]... };
	}
	template <size_t ...N>
	constexpr std::array<unsigned int, NUM_SKILLS> MakeSkillWeightList(std::index_sequence<N...>) {
		return { std::get<1>(g_SkillList[N])... };
	}
	auto SKILL_LIST = MakeSkillList(std::make_index_sequence<NUM_SKILLS>{});

	std::pair<byte, std::shared_ptr<CZombieZSkill_Base>> ZSkillFactory(CBasePlayer* player, std::function<bool(ZombieZSkillSkillId)> is_available)
	{
		std::vector<std::tuple<byte, unsigned int, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)>> list{ SKILL_LIST.begin(), SKILL_LIST.end() };
		list.erase(std::remove_if(list.begin(), list.end(), [&](std::tuple<byte, unsigned int, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)> skill) { return !is_available((ZombieZSkillSkillId)get<0>(skill)); }), list.end());

		size_t size = list.size();
		if (!size)
			return {};

		std::vector<unsigned int> weight;
		for (size_t i = 0; i < size; i++)
		{
			weight.push_back(std::get<1>(list[i]));
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		const std::size_t N = std::discrete_distribution<>(weight.begin(), weight.end())(gen);
		return { std::get<0>(list[N]), std::get<2>(list[N])(player) };
	}
};