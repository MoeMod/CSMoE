
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "gamemode/zbz/zbz_const.h"
#include "gamemode/zbz/zbz_skill.h"
#include <dlls\weapons\KnifeAttack.h>

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
		char* GetName() const override { return "破甲弹头"; }
		char* GetDesc() const override { return "增加武器的射击威力。"; } // 射击武器追加（10+1*回合数）的伤害
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
		char* GetName() const override { return "致命一击"; }
		char* GetDesc() const override { return "低概率触发触发致命一击。"; } // 有（4%+0.5%*回合数）的概率追加（100%+2%*回合数）的伤害
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
		char* GetName() const override { return "格斗大师"; };
		char* GetDesc() const override { return "近战武器加强。"; }; // 近身武器攻击时获得一个（80units60°）的攻击，伤害为（120+40*回合数），击退为（600+30*回合数），定身为（20%+3%*回合数），最大定身50%加成
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
		char* GetName() const override { return "二段跳"; }
		char* GetDesc() const override { return "在空中可以再跳一次。"; }
		ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
		void OnGetSkill() override {
			SET_PHYSICS_KEY_VALUE(m_pPlayer->edict(), "dbj", "1");
		}
	};

	class ZSkill_Health : public CZombieZSkill_Base
	{
	public:
		explicit ZSkill_Health(CBasePlayer* player) : CZombieZSkill_Base(player) {}
		char* GetName() const override { return "生命补液"; }
		char* GetDesc() const override { return "增加生命值。"; } // （400+200*回合数）
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
		char* GetName() const override { return "炸弹背包"; }
		char* GetDesc() const override { return "一定周期自动生成手雷，每（30秒-0.5秒*回合数）生成（1+1*回合数/6）个高爆手雷"; }
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
		char* GetName() const override { return "爆炸弹头"; }
		char* GetDesc() const override { return "每（30-4*回合数/3）次射击时产生爆炸，爆炸范围64，爆炸的伤害为武器本身的（35%+5%*回合数）"; }
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


	template<class T>
	std::shared_ptr<CZombieZSkill_Base> MakeHumanSkill(CBasePlayer* player)
	{
		return std::make_shared<T>(player);
	}
	const std::map<byte, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)> g_SkillList = {
		{1, MakeHumanSkill<ZSkill_BulletDamage>},
		{2, MakeHumanSkill<ZSkill_Critical>},
		{3, MakeHumanSkill<ZSkill_KnifeMaster>},
		{4, MakeHumanSkill<ZSkill_DoubleJump>},
		{5, MakeHumanSkill<ZSkill_Health>},
		{6, MakeHumanSkill<ZSkill_BombBackpack>},
		{7, MakeHumanSkill<ZSkill_ExplodeBullet>},
	};

	std::pair<byte, std::shared_ptr<CZombieZSkill_Base>> HumanSkillFactory(CBasePlayer* player, const std::set<byte>& skillIDSet)
	{
		std::vector<std::pair<byte, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)>> list{ g_SkillList.begin(), g_SkillList.end() };
		list.erase(std::remove_if(list.begin(), list.end(), [&](std::pair<byte, std::shared_ptr<CZombieZSkill_Base>(*)(CBasePlayer*)> skillLs) { return skillIDSet.find(skillLs.first) != skillIDSet.end(); }), list.end());
		auto size = list.size();
		if (!size)
			return {};

		std::random_device rd;
		const std::size_t N = std::uniform_int_distribution<size_t>(0, list.size() - 1)(rd);
		return { list[N].first, list[N].second(player) };
	}
};