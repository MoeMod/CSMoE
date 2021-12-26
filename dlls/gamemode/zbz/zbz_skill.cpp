
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

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

	/*class ZSkill_Kangaroo : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Kangaroo(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		char* GetName() const override { return "袋鼠"; }
		char* GetDesc() const override { return "增加跳跃力（30+5*回合数）"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnGetSkill() override {
			const char* pExtraPhysics = g_engfuncs.pfnGetPhysicsKeyValue(m_pPlayer->edict(), "ext");
			int flags = UTIL_ReadFlags(pExtraPhysics);
			flags |= (1 << 1);
			char szFlags[32];
			UTIL_GetFlags(szFlags, flags);
			SET_PHYSICS_KEY_VALUE(m_pPlayer->edict(), "ext", szFlags);
		}
	};*/

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
		const char* GetDesc() const override { return "增加移动速度。"; } // （5+2*回合数）
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }

		void OnResetMaxSpeed() override {
			m_pPlayer->pev->maxspeed += GetBonus();
		}
		void OnGetSkill() override {
			m_pPlayer->ResetMaxSpeed();
		}
	private:
		float GetBonus() const { return min(5.0f + 2.0f * g_pGameRules->m_iTotalRoundsPlayed, 300.0f); }
	};

	class ZSkill_Icarus : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Icarus(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		const char* GetName() const override { return "伊卡洛斯"; }
		const char* GetDesc() const override { return "持续按下跳跃键，短时间内可以滑翔。跳跃过程中碰撞到其他物体时无法启动。"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }

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
		{ Skill_DoubleJump, 100, MakeSkill<ZSkill_DoubleJump> },
		{ Skill_Health, 100, MakeSkill<ZSkill_Health> },
		{ Skill_BombBackpack, 100, MakeSkill<ZSkill_BombBackpack> },
		{ Skill_ExplodeBullet, 100, MakeSkill<ZSkill_ExplodeBullet> },
		{ Skill_Speed, 100, MakeSkill<ZSkill_Speed> },
		{ Skill_Icarus, 100, MakeSkill<ZSkill_Icarus> },
		{ Skill_Fanatic, 100, MakeSkill<ZSkill_Fanatic> },
		// Heroic Presence 英雄出现
		//{ 10, 100, MakeSkill<ZSkill_Icarus> }, // 合金利爪
		//{ 11, 100, MakeSkill<ZSkill_Icarus> }, // 钢铁防御
		// 坚持不懈 受到伤害时低概率忽略定身和击退。但伤害不会减轻
		// 接触感染 即使不攻击只要身体接触就可以感染。
		// 爆弹兽颅 受到伤害后生成爆弹兽颅。
		// 兽颅强化 增加爆弹兽颅的攻击力和击退效果。
		// 复活 获得复活能力。病毒被驱散后，致命击杀死亡时依旧不能复活。
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