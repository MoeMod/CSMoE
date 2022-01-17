//
// Created by 小白白 on 2021/12/12.
//
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "bmodels.h"
#include "triggers.h"
#include "entity_state.h"
#include "usercmd.h"

#include "mod_z4.h"

#include "util/u_range.hpp"

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>

#include <random>
#include <numeric>

#include "gamemode/z4/z4_zclass.hpp"
#include "gamemode/z4/z4_const.h"

template<class...Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class...Ts> overloaded(Ts...) -> overloaded<Ts...>;

static const char* s_szCountDownSound[11] = {
		"", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten"
};

namespace sv {
	namespace z4 {
	// DayNight Effect : Dusk & Dark & Dawn
	constexpr const char *LIGHTSTYLE[7] = { "g", "f", "e", "d", "c", "b", "b" };
	constexpr float SKY_COLOR[7] = { 0.7, 0.6, 0.6, 0.5, 0.5, 0.4, 0.4 };
	constexpr const char *FOG_DENSITY[7] = { "0.001", "0.0015", "0.002", "0.0025", "0.0025", "0.003", "0.003" };
	constexpr const char *FOG_COLOR[7] = { "0 0 0", "0 0 0", "0 0 0", "0 0 0", "0 0 0", "0 0 0", "0 0 0" };

	constexpr duration_t SKILL_ADRSHOOT_LASTING = 4.0s;
	constexpr duration_t SKILL_HUMANDASH_DAMAGE_MULTIPLY = 3.0s;
	constexpr duration_t SKILL_HUMANDASH_LASTING = 10.0s;
	constexpr duration_t SKILL_HUMANDASH_SPEED = 350.0s;

	enum DayNightStatus_e {
		DNS_DUSK,
		DNS_DARK,
		DNS_DAWN
	};
	}
	using namespace z4;

	using timer_type = boost::asio::basic_waitable_timer<EngineClock>;

	struct CMod_ZombieIV::round_t
	{
		explicit round_t(CMod_ZombieIV *p) : mod(p) {
			m_iSkyColor[0] = CVAR_GET_FLOAT("sv_skycolor_r");
			m_iSkyColor[1] = CVAR_GET_FLOAT("sv_skycolor_g");
			m_iSkyColor[2] = CVAR_GET_FLOAT("sv_skycolor_b");

			boost::asio::co_spawn(ioc, Task_Night(), boost::asio::detached);
		}

		CMod_ZombieIV * const mod;
		boost::asio::io_context ioc;

		int m_iSkyColor[3];

		int m_iGameLight;

		DayNightStatus_e m_iDayNightStatus;

		void Think()
		{
			ioc.poll();
		}

		boost::asio::awaitable<void> Task_Night()
		{
			try
			{
				m_iDayNightStatus = DNS_DUSK;

				timer_type timer(ioc);

				// countdown
				timer.expires_after(20s);
				co_await timer.async_wait(boost::asio::use_awaitable);

				auto night_start_time = RandomDuration(mod->m_iRoundTimeSecs / 5, mod->m_iRoundTimeSecs / 2);
				timer.expires_after(night_start_time);
				co_await timer.async_wait(boost::asio::use_awaitable);

				// night starts, pre 15secs
				for (CBasePlayer* player : moe::range::PlayersList())
					CLIENT_COMMAND(player->edict(), "spk z4/z4_bgm_night.wav\n");

				for(int iLight = 0; iLight < 5; ++iLight)
				{
					Update_Environment(iLight);
					timer.expires_after(3s);
					co_await timer.async_wait(boost::asio::use_awaitable);
				}
				m_iDayNightStatus = DNS_DARK;
				Check_NightStart();

				// night running for 45secs
				timer.expires_after(45s);
				co_await timer.async_wait(boost::asio::use_awaitable);

				for(int iLight = 6; iLight > 0; --iLight)
				{
					Update_Environment(iLight);

					timer.expires_after(3s);
					co_await timer.async_wait(boost::asio::use_awaitable);
				}

				// night ends
				for (CBasePlayer* player : moe::range::PlayersList())
					CLIENT_COMMAND(player->edict(), "spk dm_darksnow/planes_bomb.wav\n");

				m_iDayNightStatus = DNS_DAWN;
				Check_NightEnd();

				// human win
			}
			catch(const boost::system::system_error & e)
			{
				if(e.code() == boost::system::errc::operation_canceled)
				{
					// ignored
				}
			}
		}

		void Update_Environment(int iLight)
		{
			m_iGameLight = iLight;
			Set_SkyColor(SKY_COLOR[iLight]);
			Set_EnvironmentFog(FOG_DENSITY[iLight], FOG_COLOR[iLight]);
			Set_Light(LIGHTSTYLE[iLight]);
		}

		void Set_SkyColor(float x = 1.0)
		{
			CVAR_SET_FLOAT("sv_skycolor_r", m_iSkyColor[0] * x);
			CVAR_SET_FLOAT("sv_skycolor_g", m_iSkyColor[1] * x);
			CVAR_SET_FLOAT("sv_skycolor_b", m_iSkyColor[2] * x);
		}

		void Set_EnvironmentFog(const char *szDensity = "0.0016", const char *szColor = "0 0 0")
		{
			mod->m_pFogEntity->m_fDensity = atof(szDensity);
			mod->m_pFogEntity->pev->rendercolor = {0, 0, 0};
		}

		void Set_Light(const char *LightStyle)
		{
			// light
			LIGHT_STYLE(0, LightStyle); // previous one is "f"
		}

		void Check_NightStart()
		{
		}

		void Check_NightEnd()
		{
		}

		DayNightStatus_e GetDayNightStatus() const { return m_iDayNightStatus; }
	};

	CMod_ZombieIV::CMod_ZombieIV() : m_CountDownDelegate(this) // precache
	{
		PRECACHE_SOUND("zombi/z4/all/z4human_death_male.wav");

		z4::Z4_ZClass_Precache_All();

		round = std::make_unique<round_t>(this);
		m_Countdown.SetDelegate(&m_CountDownDelegate);
	}

	void CMod_ZombieIV::UpdateGameMode(CBasePlayer *pPlayer)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
		WRITE_BYTE(MOD_ZB4);
		WRITE_BYTE(0); // Reserved. (weapon restriction? )
		WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
		WRITE_BYTE(0); // Reserved. (MaxTime?)

		MESSAGE_END();
	}

	void CMod_ZombieIV::RestartRound()
	{
		round = nullptr;
		round = std::make_unique<round_t>(this);
		return CMod_Zombi::RestartRound();
	}

	void CMod_ZombieIV::Think()
	{
		if(round)
			round->Think();
		return CMod_Zombi::Think();
	}

	void CMod_ZombieIV::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
	{
		return CMod_Zombi::HumanInfectionByZombie(player, attacker);
	}

	CMod_ZombieIV::~CMod_ZombieIV()
	{

	}

	class IZombieModeCharacter_Z4 : virtual public IZombieModeCharacter
	{
	public:
		virtual void OnWeaponDeploy(CBasePlayerItem *item) = 0;
		virtual void Skill_Dash_Start() = 0;
		virtual void Skill_Dash_End() = 0;
		virtual void Skill_Activate() = 0;
	};

	class CPlayerModStrategy_ZB4 : public CPlayerModStrategy_ZB1
	{
	public:
		CPlayerModStrategy_ZB4(CBasePlayer *player, CMod_ZombieIV *mp)
				:	CPlayerModStrategy_ZB1(player, mp),
				     m_pModZB4(mp), work_guard(boost::asio::make_work_guard(ioc))
		{}
		~CPlayerModStrategy_ZB4() = default;
	public:
		void BecomeZombie(ZombieLevel iEvolutionLevel) override;

		void BecomeHuman() override;

		void OnSpawn() override;

		void OnThink() override;

		BOOL DeployWeapon(CBasePlayerItem *item) override
		{
			BOOL ret = CPlayerModStrategy_Default::DeployWeapon(item);
			if(m_pCharacter_Z4)
				m_pCharacter_Z4->OnWeaponDeploy(item);
			return ret;
		}

		void CmdStart(struct usercmd_s *cmd, unsigned int random_seed) override;

		void GiveDefaultItems() override;
		bool ClientCommand(const char *pcmd) override;
		float AdjustDamageGivenToPlayer(entvars_t *pevInflictor, CBasePlayer *victim, float flDamage, int bitsDamageType) override;

	public:
		bool IsZombie() const { return m_pPlayer->m_bIsZombie; }
		bool IsAlive() const { return m_pPlayer->IsAlive(); }


	public:
		void OnPressW()
		{
			if(m_bDashActivateCheckRunning)
			{
				w_press_timer.cancel();
			}
			else
			{
				boost::asio::co_spawn(ioc, Task_DashActivateCheck(), boost::asio::detached);
			}
		}

		void OnReleaseW()
		{
			if(m_bDashActivateCheckRunning)
			{
				w_release_timer.cancel();
			}
		}

		boost::asio::awaitable<void> Task_DashActivateCheck()
		{
			m_bDashActivateCheckRunning = true;
			try {
				w_release_timer.expires_after(0.15s);
				try {
					co_await w_release_timer.async_wait(boost::asio::use_awaitable);
					m_bDashActivateCheckRunning = false;
					co_return;
				} catch(const boost::system::system_error & e) {  }


				w_press_timer.expires_after(0.15s);
				try {
					co_await w_press_timer.async_wait(boost::asio::use_awaitable);
					m_bDashActivateCheckRunning = false;
					co_return;
				} catch(const boost::system::system_error & e) {  }

				if(m_pCharacter_Z4)
					m_pCharacter_Z4->Skill_Dash_Start();
			}
			catch(const boost::system::system_error & e)
			{
				if(e.code() == boost::system::errc::operation_canceled)
				{
					// ignored
				}
			}
			m_bDashActivateCheckRunning = false;
			co_return;
		}

	public:
		CMod_ZombieIV * const m_pModZB4;
		boost::asio::io_context ioc;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;
		bool m_bHasDashActivateCheck = false;
		struct adrenaline_t;
		std::unique_ptr<adrenaline_t> adrenaline;
		std::shared_ptr<IZombieModeCharacter_Z4> m_pCharacter_Z4;

		bool m_bDashActivateCheckRunning = false;
		timer_type w_press_timer{ioc};
		timer_type w_release_timer{ioc};
	};

	struct CPlayerModStrategy_ZB4::adrenaline_t
	{
		adrenaline_t(CMod_ZombieIV *p, CPlayerModStrategy_ZB4 *s) : mod(p), strategy(s) {
			boost::asio::co_spawn(ioc, Task_Adrenaline_Check(), boost::asio::detached);
		}

		CMod_ZombieIV * const mod;
		CPlayerModStrategy_ZB4 * const strategy;
		boost::asio::io_context ioc;

		int m_iAdrenaline = 0;
		bool m_bIsUsingAdrShoot = false;
		bool m_bIsUsingDash = false;
		bool m_bIsSlowDown = false;

		void Think()
		{
			ioc.poll();
		}

		boost::asio::awaitable<void> Task_Adrenaline_Check()
		{
			try {
				timer_type timer(ioc);
				auto dns = mod->round->GetDayNightStatus();

				while(true)
				{
					while(m_bIsUsingAdrShoot || m_bIsUsingDash)
					{
						timer.expires_after(1s);
						co_await timer.async_wait(boost::asio::use_awaitable);
					}

					if(strategy->IsAlive())
					{
						if(dns == DNS_DARK && strategy->IsZombie())
						{
							Adrenaline_Set(7, 1.0f);
						}
						else
						{
							Adrenaline_Set(1);
						}
					}

					timer.expires_after(1s);
					co_await timer.async_wait(boost::asio::use_awaitable);
				}
			}
			catch(const boost::system::system_error & e)
			{
				if(e.code() == boost::system::errc::operation_canceled)
				{
					// ignored
				}
			}
		}

		void Adrenaline_Set(int DeltaValue, float flFlashTime = 0.5f)
		{
			int oldvalue = std::exchange(m_iAdrenaline, std::clamp(m_iAdrenaline + DeltaValue, 0, 120));
			if(m_iAdrenaline != oldvalue)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgZ4Msg, nullptr, strategy->m_pPlayer->pev);
				WRITE_BYTE(Z4_MESSAGE_ENERGY);
				WRITE_BYTE(m_iAdrenaline);
				WRITE_BYTE(flFlashTime * 10);
				MESSAGE_END();
			}

		}

		void Skill_AdrShoot_Activate()
		{
			if(m_bIsUsingAdrShoot)
				return;
			if(m_iAdrenaline < 30)
				return;

			auto time = LerpDuration(zero_duration, SKILL_ADRSHOOT_LASTING, m_iAdrenaline / 100.0);
			m_bIsUsingAdrShoot = true;
			boost::asio::co_spawn(ioc, Task_Adrenaline_SkillConsume(), boost::asio::detached);
			// TODO : fire message
		}

		void Skill_AdrShoot_End()
		{
			if(!m_bIsUsingAdrShoot)
				return;
			m_bIsUsingAdrShoot = false;
			// TODO : fire message
		}

		boost::asio::awaitable<void> Task_Adrenaline_SkillConsume()
		{
			try {
				timer_type timer(ioc);
				auto dns = mod->round->GetDayNightStatus();

				while(m_iAdrenaline > 0)
				{
					if(m_bIsUsingAdrShoot)
					{
						Adrenaline_Set(-5);

						timer.expires_after(SKILL_ADRSHOOT_LASTING / 20);
						co_await timer.async_wait(boost::asio::use_awaitable);
						continue;
					}
				}

				Skill_AdrShoot_End();
			}
			catch(const boost::system::system_error & e)
			{
				if(e.code() == boost::system::errc::operation_canceled)
				{
					// ignored
				}
			}
		}
	};

	class CZombie_Z4 : public CZombie_ZB1, virtual public IZombieModeCharacter_Z4
	{
	public:
		CZombie_Z4(CPlayerModStrategy_ZB4 *strategy, ZombieLevel lv) : CZombie_ZB1(strategy->m_pPlayer, lv),
			strategy(strategy),
			zclass(z4::Z4_MakeRandomZombie(m_pPlayer))
		{
			boost::asio::co_spawn(ioc, Task_Dash_SkillConsume(), boost::asio::detached);
			boost::asio::co_spawn(ioc, Task_Dash_SkillCheck(), boost::asio::detached);
		}

		float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override { return flDamage; }
		void ResetMaxSpeed() const override
		{
			auto ability = AbilityForZClass();
			m_pPlayer->pev->maxspeed = strategy->adrenaline->m_bIsUsingDash ? ability.flDashSpeed : ability.flSpeed;
			m_pPlayer->pev->gravity = strategy->adrenaline->m_bIsUsingDash ? ability.flDashGravity : ability.flGravity;
		}

		void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override
		{
			z4::zclass_sound_s soundlist = SoundForZClass();
			if(RANDOM_LONG(0, 1))
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, soundlist.szSoundPain1, VOL_NORM, ATTN_NORM);
			else
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, soundlist.szSoundPain2, VOL_NORM, ATTN_NORM);
		}

		void DeathSound_Zombie() override
		{
			z4::zclass_sound_s soundlist = SoundForZClass();
			if(RANDOM_LONG(0, 1))
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, soundlist.szSoundDeath1, VOL_NORM, ATTN_NORM);
			else
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, soundlist.szSoundDeath2, VOL_NORM, ATTN_NORM);
		}

		void OnWeaponDeploy(CBasePlayerItem *item) override
		{
			std::visit([item, player = m_pPlayer](auto &zc) {

				switch(item->m_iId) {
					case WEAPON_KNIFE: {
						player->pev->viewmodel = MAKE_STRING(zc.V_Model);
						break;
					}
					default: {
						// assert(false)
						break;
					}
				}
			}, zclass);
		}

		void Think() override
		{
			ioc.poll();
			return std::visit([](auto &zc) {
				return zc.OnThink();
			}, zclass);
		}

		z4::zclass_sound_s SoundForZClass() const
		{
			return std::visit([](auto &zc) {
				return zc.Sound;
			}, zclass);
		}

		z4::zclass_ability_s AbilityForZClass() const
		{
			return std::visit([](auto &zc) {
				return zc.Ability;
			}, zclass);
		}

		z4::zclass_skill_s SkillForZClass() const
		{
			return std::visit([](auto &zc) {
				return zc.Skill;
			}, zclass);
		}

		boost::asio::awaitable<void> Task_Dash_SkillConsume()
		{
			try {
				auto skill = SkillForZClass();
				timer_type timer(ioc);

				while(true)
				{
					timer.expires_after(skill.flDashLasting / 20);
					co_await timer.async_wait(boost::asio::use_awaitable); // throws

					if(strategy->adrenaline->m_bIsUsingDash)
					{
						strategy->adrenaline->Adrenaline_Set(-5);

						if(strategy->adrenaline->m_iAdrenaline <= 0)
							Skill_Dash_End();
					}
				}
			}
			catch(const boost::system::system_error & e)
			{
				if(e.code() == boost::system::errc::operation_canceled)
				{
					// ignored
				}
			}
		}

		time_point_t m_flNextSkillActivate = invalid_time_point;

		void Skill_Activate() override
		{
			if(strategy->adrenaline->m_bIsUsingAdrShoot || strategy->adrenaline->m_bIsUsingDash)
				return;
			if(strategy->adrenaline->m_bIsSlowDown)
				return;
			if(gpGlobals->time < m_flNextSkillActivate)
				return;

			auto Skill = SkillForZClass();
			if(strategy->adrenaline->m_iAdrenaline < Skill.iSkillConsume)
				return;

			std::visit([](auto &zc) {
				return zc.OnSkillActivate();
			}, zclass);

			m_flNextSkillActivate = gpGlobals->time + Skill.flSkillTimeReset;
			strategy->adrenaline->Adrenaline_Set(-Skill.iSkillConsume);
		}

		boost::asio::awaitable<void> Task_Dash_SkillCheck()
		{
			try {
				auto skill = SkillForZClass();
				auto soundlist = SoundForZClass();
				timer_type timer(ioc);

				while(true)
				{
					timer.expires_after(skill.flDashTimeCheck);
					co_await timer.async_wait(boost::asio::use_awaitable); // throws

					if(strategy->adrenaline->m_bIsUsingDash)
					{
						std::visit([](auto &zc) {
							return zc.OnDashCheck();
						}, zclass);

						m_pPlayer->ResetMaxSpeed();

						auto skill = SkillForZClass();
						CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
						if (pWeapon)
						{
							pWeapon->SendWeaponAnim(ANIM_DASH_LOOP, TRUE);
							pWeapon->m_flTimeWeaponIdle = skill.flDashTimeCheck;
						}

						if(RANDOM_LONG(0, 1))
							EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, soundlist.szSoundDash1, VOL_NORM, ATTN_NORM);
						else
							EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, soundlist.szSoundDash2, VOL_NORM, ATTN_NORM);

						m_pPlayer->m_iCustomSequence = m_pPlayer->LookupSequence("dash");
						m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + skill.flDashTimeCheck;
					}
				}
			}
			catch(const boost::system::system_error & e)
			{
				if(e.code() == boost::system::errc::operation_canceled)
				{
					// ignored
				}
			}
		}

		void Skill_Dash_Start() override
		{
			if(strategy->adrenaline->m_bIsUsingAdrShoot || strategy->adrenaline->m_bIsUsingDash)
				return;
			if(strategy->adrenaline->m_iAdrenaline <= 0)
				return;
			if(strategy->adrenaline->m_bIsSlowDown)
				return;

			strategy->adrenaline->m_bIsUsingDash = true;

			std::visit([](auto &zc) {
				return zc.OnDashStart();
			}, zclass);

			m_pPlayer->ResetMaxSpeed();

			auto skill = SkillForZClass();
			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
			if (pWeapon)
			{
				pWeapon->SendWeaponAnim(ANIM_DASH_START, TRUE);
				pWeapon->m_flTimeWeaponIdle = skill.flDashTimeCheck;
			}

		}

		void Skill_Dash_End() override
		{
			if(!strategy->adrenaline->m_bIsUsingDash)
				return;
			// TODO
			strategy->adrenaline->m_bIsUsingDash = false;

			std::visit([](auto &zc) {
				return zc.OnDashEnd();
			}, zclass);

			m_pPlayer->ResetMaxSpeed();

			auto skill = SkillForZClass();
			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
			if (pWeapon)
			{
				pWeapon->SendWeaponAnim(ANIM_DASH_END, TRUE);
				pWeapon->m_flTimeWeaponIdle = skill.flDashTimeCheck;
			}
		}

	public:
		CPlayerModStrategy_ZB4 *strategy;
		z4::zclass_variant_t zclass;
		boost::asio::io_context ioc;
	};

	class CHuman_Z4 : public CHuman_ZB1, virtual public IZombieModeCharacter_Z4
	{
	public:
		CHuman_Z4(CPlayerModStrategy_ZB4 *strategy) : CHuman_ZB1(strategy->m_pPlayer),
		strategy(strategy)
		{

		}

	public:
		CPlayerModStrategy_ZB4 *strategy;
	};

	void CMod_ZombieIV::InstallPlayerModStrategy(CBasePlayer *player)
	{
		std::unique_ptr<CPlayerModStrategy_ZB4> up = std::make_unique<CPlayerModStrategy_ZB4>(player, this);
		player->m_pModStrategy = std::move(up);
	}

	void CMod_ZombieIV::InfectionSound()
	{
		UTIL_BroadcastStuffText("spk \"z4/z4_zombi_coming_%d\"\n", RANDOM_LONG(1, 2));
	}

	void CMod_ZombieIV::EmitHomoSound(CBasePlayer *player, CBasePlayer *attacker)
	{
		EMIT_SOUND(ENT(player->pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "zombi/z4/all/z4human_death_male.wav" : "zombi/z4/all/z4human_death_male.wav", VOL_NORM, ATTN_NORM);
	}

	void CMod_ZombieIV::HumanWin()
	{
		UTIL_BroadcastStuffText("spk \"z4/z4_win_human.wav\"\n");

		/*MESSAGE_BEGIN(MSG_ALL, gmsgShowWin);
		WRITE_BYTE(2);
		WRITE_BYTE(5);
		MESSAGE_END();*/

		EndRoundMessage("--------『人类胜利』--------", ROUND_CTS_WIN);
		TerminateRound(5s, WINSTATUS_CTS);
		RoundEndScore(WINSTATUS_CTS);

		++m_iNumCTWins;
		UpdateTeamScores();
	}

	void CMod_ZombieIV::ZombieWin()
	{
		//Broadcast("terwin");
		for(CBasePlayer *player : moe::range::PlayersList())
			CLIENT_COMMAND(player->edict(), "spk z4/z4_win_zombie.wav\n");

		/*MESSAGE_BEGIN(MSG_ALL, gmsgShowWin);
		WRITE_BYTE(3);
		WRITE_BYTE(5);
		MESSAGE_END();*/

		EndRoundMessage("--------『僵尸胜利』--------", ROUND_TERRORISTS_WIN);
		TerminateRound(5s, WINSTATUS_TERRORISTS);
		RoundEndScore(WINSTATUS_TERRORISTS);

		++m_iNumTerroristWins;
		UpdateTeamScores();
	}

	void CPlayerModStrategy_ZB4::OnThink()
	{
		CPlayerModStrategy_ZB1::OnThink();
		if(adrenaline)
			adrenaline->Think();
		ioc.poll();
	}

	void CPlayerModStrategy_ZB4::GiveDefaultItems()
	{
		if(m_pPlayer->m_bIsZombie)
		{
			m_pPlayer->RemoveAllItems(FALSE);
			m_pPlayer->m_bHasPrimary = false;

			m_pPlayer->GiveNamedItem("knife_zombi_z4");


			if (!(m_pPlayer->m_flDisplayHistory & DHF_NIGHTVISION))
			{
				m_pPlayer->HintMessage("#Hint_use_nightvision");
				m_pPlayer->m_flDisplayHistory |= DHF_NIGHTVISION;
			}
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "items/equip_nvg.wav", VOL_NORM, ATTN_NORM);
			m_pPlayer->m_bHasNightVision = true;
			SendItemStatus(m_pPlayer);
		}
		else
		{
			return CPlayerModStrategy_ZB1::GiveDefaultItems();
		}
	}

	bool CPlayerModStrategy_ZB4::ClientCommand(const char *pcmd)
	{
		if (!m_pPlayer->m_bIsZombie)
		{
			if (!Q_stricmp(pcmd, "z4_skill5"))
			{
				// TODO : activate kick
				return true;
			}
			else if (!Q_stricmp(pcmd, "z4_skill6"))
			{
				adrenaline->Skill_AdrShoot_Activate();
				return true;
			}
		}
		else
		{
			if (!Q_stricmp(pcmd, "z4_zombieskill"))
			{
				m_pCharacter_Z4->Skill_Activate();
				return true;
			}
		}

		return false;
	}

	void CPlayerModStrategy_ZB4::BecomeZombie(ZombieLevel iEvolutionLevel)
	{
		auto p = std::make_shared<CZombie_Z4>(this, iEvolutionLevel);
		m_pCharacter = p;
		m_pCharacter_Z4 = p;
		adrenaline = std::make_unique<adrenaline_t>(m_pModZB4, this);
		adrenaline->Adrenaline_Set(100, 0.2f);

		DeployWeapon(m_pPlayer->m_pActiveItem);
	}

	void CPlayerModStrategy_ZB4::BecomeHuman()
	{
		m_pCharacter = std::make_shared<CHuman_ZB1>(m_pPlayer);
		m_pCharacter_Z4 = nullptr; // TODO
		adrenaline = std::make_unique<adrenaline_t>(m_pModZB4, this);
		adrenaline->Adrenaline_Set(0, 0.0f);
	}

	void CPlayerModStrategy_ZB4::OnSpawn()
	{
		return CPlayerModStrategy_ZB1::OnSpawn();
	}

	float CPlayerModStrategy_ZB4::AdjustDamageGivenToPlayer(entvars_t *pevInflictor, CBasePlayer *victim, float flDamage, int bitsDamageType)
	{
		// morale calc
		if(!m_pPlayer->m_bIsZombie)
		{
			if(adrenaline->m_bIsUsingAdrShoot)
				flDamage *= 3.0f;
			else
				flDamage *= 1.0f + (adrenaline->m_iAdrenaline / 100.0f);
		}
		return flDamage;
	}

	void CPlayerModStrategy_ZB4::CmdStart(usercmd_t *cmd, unsigned int random_seed)
	{
		CPlayerModStrategy_Default::CmdStart(cmd, random_seed);
		if(cmd->buttons & IN_FORWARD)
		{
			OnPressW();
		}
		else
		{
			OnReleaseW();
		}
		if(!(cmd->buttons & IN_FORWARD) || (cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_DUCK) )
		{
			if(m_pCharacter_Z4)
				m_pCharacter_Z4->Skill_Dash_End();
		}
	}

	CZ4CountdownDelegate::CZ4CountdownDelegate(CMod_ZombieIV *mod) : CZB1CountdownDelegate(mod) {}

	void CZ4CountdownDelegate::OnCountdownStart()
	{
		for (CBasePlayer* player : moe::range::PlayersList())
			CLIENT_COMMAND(player->edict(), "spk z4/z4_bgm_resist.wav\n");
	}

	void CZ4CountdownDelegate::OnCountdownChanged(int iCurrentCount)
	{
		static const char* s_szCountDownSound[11] = {
				"",
				"z4/z4_count_one.wav",
				"z4/z4_count_two.wav",
				"z4/z4_count_three.wav",
				"z4/z4_count_four.wav",
				"z4/z4_count_five.wav",
				"z4/z4_count_six.wav",
				"z4/z4_count_seven.wav",
				"z4/z4_count_eight.wav",
				"z4/z4_count_nine.wav",
				"z4/z4_count_ten.wav"
		};

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "生化病毒正在寻找宿主，%s1秒后出现母体僵尸", UTIL_dtos1(iCurrentCount)); // #CSO_ZombiSelectCount
		if (iCurrentCount <= 10)
		{
			for (CBasePlayer* player : moe::range::PlayersList())
				CLIENT_COMMAND(player->edict(), "spk %s\n", s_szCountDownSound[iCurrentCount]);

		}
	}
}
