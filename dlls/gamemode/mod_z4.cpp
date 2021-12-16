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

#include "mod_z4.h"

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>

#include <random>
#include <numeric>

#include "gamemode/z4/z4_zclass.hpp"

template<class...Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class...Ts> overloaded(Ts...) -> overloaded<Ts...>;

static const char* s_szCountDownSound[11] = {
		"", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten"
};

namespace sv {

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
		PRECACHE_SOUND("zombi/zombi_box.wav");
		PRECACHE_SOUND("zombi/zombi_evolution.wav");
		PRECACHE_SOUND("zombi/zombi_evolution_female.wav");

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
	};

	class CPlayerModStrategy_ZB4 : public CPlayerModStrategy_ZB1
	{
	public:
		CPlayerModStrategy_ZB4(CBasePlayer *player, CMod_ZombieIV *mp)
				:	CPlayerModStrategy_ZB1(player, mp),
				     m_pModZB4(mp)
		{}
		~CPlayerModStrategy_ZB4() = default;
	public:
		void BecomeZombie(ZombieLevel iEvolutionLevel) override;

		void BecomeHuman() override;

		void OnSpawn() override;

		void OnThink() override;

		BOOL DeployWeapon(CBasePlayerItem *item) override
		{
			CPlayerModStrategy_Default::DeployWeapon(item);
			if(m_pCharacter_Z4)
				m_pCharacter_Z4->OnWeaponDeploy(item);
		}

		void GiveDefaultItems() override;
		bool ClientCommand(const char *pcmd) override;
		float AdjustDamageGivenToPlayer(entvars_t *pevInflictor, CBasePlayer *victim, float flDamage, int bitsDamageType) override;

	public:
		bool IsZombie() const { return m_pPlayer->m_bIsZombie; }
		bool IsAlive() const { return m_pPlayer->IsAlive(); }

	public:
		CMod_ZombieIV * const m_pModZB4;
		struct adrenaline_t;
		std::unique_ptr<adrenaline_t> adrenaline;
		std::shared_ptr<IZombieModeCharacter_Z4> m_pCharacter_Z4;
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
						// wait for next frame
						co_await boost::asio::post(ioc, boost::asio::use_awaitable);
					}

					if(strategy->IsAlive())
					{
						if(dns == DNS_DARK && strategy->IsZombie())
						{
							Adrenaline_Add(7);
						}
						else
						{
							Adrenaline_Add(1);
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

		void Adrenaline_Add(int DeltaValue)
		{
			int oldvalue = std::exchange(m_iAdrenaline, std::clamp(m_iAdrenaline + DeltaValue, 0, 120));
			if(m_iAdrenaline != oldvalue)
			{
				// TODO : fire message
			}

		}

		void Skill_AdrShoot_Activate()
		{
			if(m_iAdrenaline < 30)
				return;

			auto time = LerpDuration(zero_duration, SKILL_ADRSHOOT_LASTING, m_iAdrenaline / 100.0);
			m_bIsUsingAdrShoot = true;
			boost::asio::co_spawn(ioc, Task_Adrenaline_SkillConsume(), boost::asio::detached);
			// TODO : fire message
		}

		void Skill_AdrShoot_End()
		{
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
					Adrenaline_Add(-5);

					timer.expires_after(SKILL_ADRSHOOT_LASTING / 20);
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
	};

	class CZombie_Z4 : public CZombie_ZB1, virtual public IZombieModeCharacter_Z4
	{
	public:
		CZombie_Z4(CPlayerModStrategy_ZB4 *strategy, ZombieLevel lv) : CZombie_ZB1(strategy->m_pPlayer, lv),
			strategy(strategy),
			zclass(z4::Z4_MakeRandomZombie(m_pPlayer))
		{

		}

		float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override { return flDamage; }
		void ResetMaxSpeed() const override
		{
			auto ability = AbilityForZClass();
			m_pPlayer->pev->maxspeed = strategy->adrenaline->m_bIsUsingDash ? ability.flDashSpeed : ability.flSpeed;
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

	public:
		CPlayerModStrategy_ZB4 *strategy;
		z4::zclass_variant_t zclass;
	};

	void CMod_ZombieIV::InstallPlayerModStrategy(CBasePlayer *player)
	{
		std::unique_ptr<CPlayerModStrategy_ZB4> up = std::make_unique<CPlayerModStrategy_ZB4>(player, this);
		player->m_pModStrategy = std::move(up);
	}

	void CPlayerModStrategy_ZB4::OnThink()
	{
		CPlayerModStrategy_ZB1::OnThink();
		if(adrenaline)
			adrenaline->Think();
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

		return false;
	}

	void CPlayerModStrategy_ZB4::BecomeZombie(ZombieLevel iEvolutionLevel)
	{
		auto p = std::make_shared<CZombie_Z4>(this, iEvolutionLevel);
		m_pCharacter = p;
		m_pCharacter_Z4 = p;
		adrenaline = std::make_unique<adrenaline_t>(m_pModZB4, this);
		adrenaline->Adrenaline_Add(100);

		DeployWeapon(m_pPlayer->m_pActiveItem);
	}

	void CPlayerModStrategy_ZB4::BecomeHuman()
	{
		m_pCharacter = std::make_shared<CHuman_ZB1>(m_pPlayer);
		m_pCharacter_Z4 = nullptr; // TODO
		adrenaline = std::make_unique<adrenaline_t>(m_pModZB4, this);
		adrenaline->Adrenaline_Add(0);
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

	CZ4CountdownDelegate::CZ4CountdownDelegate(CMod_ZombieIV *mod) : CZB1CountdownDelegate(mod) {}

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
