
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "entity_state.h"

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
		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;
			if (m_pPlayer->m_pActiveItem->iItemSlot() != KNIFE_SLOT)
				return;
			if (m_flNextThink > gpGlobals->time)
				return;
			m_flNextThink = gpGlobals->time + 0.01s;

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			auto result = KnifeAttack3(vecSrc, gpGlobals->v_forward, 120.0f + 40.0f * g_pGameRules->m_iTotalRoundsPlayed, 80, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
			if (result == HIT_NONE)
				return;
			/*if (result == HIT_PLAYER) {
				ApplyKnockbackData(victim, victim->pev->origin - m_pPlayer->pev->origin, { 700, 1600, 1300, 400, 0.5f });
			}*/
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
	};

	class ZSkill_Health : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Health(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "生命补液"; }
		const char* GetDesc() const override { return "增加生命值。"; } // （400+200*回合数）
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnBecomeHuman() override {
			OnGetSkill();
		}
		void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {
			OnGetSkill();
		}
		void OnGetSkill() override {
			m_pPlayer->pev->health += GetBonus();
		}
	private:
		float GetBonus() const { return 400.0f + 200.0f * g_pGameRules->m_iTotalRoundsPlayed; }
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
		const char* GetDesc() const override { return "每（30-4*回合数/3）次射击时产生爆炸，爆炸范围64，爆炸的伤害为武器本身的（35%+5%*回合数）"; }
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
			RadiusDamage3(ptr->vecEndPos, m_pPlayer->pev, m_pPlayer->pev, flDamage * 0.35f + 0.05f * g_pGameRules->m_iTotalRoundsPlayed, 64.0f, CLASS_MACHINE, DMG_EXPLOSION, 0, 0, 0, 0);

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
			if (m_pPlayer->pev->groundentity == other->edict())
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
				data.flFlying = data.flDucking = data.flNotOnGround = data.flVelocityModifier = 0.0f;

			if (RANDOM_FLOAT(0, 1) > 0.01f + 0.005f * g_pGameRules->m_iTotalRoundsPlayed)
				return;

			data.flFlying = data.flDucking = data.flNotOnGround = data.flVelocityModifier = 0.0f;
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
		float GetBonus() const { return 4.0f + 2.0f * g_pGameRules->m_iTotalRoundsPlayed; }
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

		void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* attacker, float& flDamage, int bitsDamageType) override {
			if (attacker != m_pPlayer)
				return;
			if (!IsActivate())
				return;

			CGrenade* grenade = GetClassPtr<CGrenade>(pevInflictor);
			if (!grenade)
				return;

			flDamage *= GetBonus();
		}
	private:
		float GetBonus() const { return 1.0f + 0.0056f * g_pGameRules->m_iTotalRoundsPlayed; }
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

			p->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, 1000.0f, DMG_BULLET | DMG_NEVERGIB);
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
				m_pPlayer->m_iZombieLevel = ZOMBIE_LEVEL_ORIGIN;
				auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(m_pPlayer->m_pModStrategy.get());
				if (strategy)
				{
					strategy->SetRagePercent(101.0f);
					strategy->CheckEvolution();
				}
			}

		}
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
			textParms.effect = 2;
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
		{ Skill_SixthSense, 100, MakeSkill<ZSkill_SixthSense> },
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

	std::pair<byte, std::shared_ptr<CZombieZSkill_Base>> ZSkillFactory(CBasePlayer* player, const std::set<byte>& skillIDSet)
	{
		std::vector<std::tuple<byte, unsigned int, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)>> list{ SKILL_LIST.begin(), SKILL_LIST.end() };
		list.erase(std::remove_if(list.begin(), list.end(), [&](std::tuple<byte, unsigned int, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)> skill) { return skillIDSet.find(std::get<0>(skill)) != skillIDSet.end(); }), list.end());

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