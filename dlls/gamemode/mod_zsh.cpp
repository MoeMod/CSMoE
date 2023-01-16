
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "bmodels.h"
#include "triggers.h"
#include "entity_state.h"

#include "mod_zsh.h"

#include "zbs/zs_subs.h"
#include "zbs/zbs_const.h"
#include "zbs/monster_entity.h"
#include "zsh/zsh_building.h"
#include "zsh/zsh_building_turret.h"
#include "zsh/zsh_building_turret_grenade.h"
#include "player/csdm_randomspawn.h"

#include "player/player_human_level.h"

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>

#include <random>
#include <numeric>
#include <algorithm>

namespace sv {
	using namespace zsh;

	template<class T>
	std::shared_ptr<CBaseBuildingClass> MakeBuildingClass(CZSHBuilding* p)
	{
		return std::make_shared<T>(p);
	}
	const std::tuple<ZSHBuildingClass, const char*, std::shared_ptr<CBaseBuildingClass>(*)(CZSHBuilding*)> g_BuildingClassList[] = {
		{BUILDING_GENERATOR_DIESEL, "1", MakeBuildingClass<CBuilding_Generator_Diesel>},
		{BUILDING_TURRET, "2", MakeBuildingClass<CBuilding_Turret>},
		{BUILDING_TURRET_GRENADE, "3", MakeBuildingClass<CBuilding_TurretGrenade>},
	};

	constexpr auto NUM_BUILDINGCLASSES = std::extent<decltype(g_BuildingClassList)>::value;

	std::shared_ptr<CBaseBuildingClass> BuildingClassFactory(ZSHBuildingClass iClass, CZSHBuilding* pBuilding)
	{
		for (auto&& pr : g_BuildingClassList)
		{
			if (iClass == std::get<0>(pr))
			{
				return std::get<2>(pr)(pBuilding);
			}
		}

		return std::get<2>(g_BuildingClassList[0])(pBuilding);
	}

	using timer_type = boost::asio::basic_waitable_timer<EngineClock>;

	struct CMod_ZombieShelter::round_t
	{
		explicit round_t(CMod_ZombieShelter* p) : mod(p), m_iDays(0){
			m_iSkyColor[0] = CVAR_GET_FLOAT("sv_skycolor_r");
			m_iSkyColor[1] = CVAR_GET_FLOAT("sv_skycolor_g");
			m_iSkyColor[2] = CVAR_GET_FLOAT("sv_skycolor_b");
			m_iMaxResource[RT_EPOWER] = 300;
			boost::asio::co_spawn(ioc, Task_Night(), boost::asio::detached);
		}

		CMod_ZombieShelter* const mod;
		boost::asio::io_context ioc;

		int m_iSkyColor[3];
		/* 亮度(0-6越大越暗) */
		int m_iGameLight;
		/* 天数 */
		unsigned int m_iDays;
		/* 天气 */
		WeatherType_e m_iWeather;
		/* 天气等级 */
		WeatherLevel_e m_iWeatherLevel;
		/* 昼夜 */
		DayNightStatus_e m_iDayNightStatus;
		/* 储存资源 */
		int m_iResource[RT_MAX];
		/* 储存最大资源 */
		int m_iMaxResource[RT_MAX];

		void Think()
		{
			ioc.poll();
		}

		boost::asio::awaitable<void> Task_Night()
		{
			try
			{
				// day start
				++m_iDays;
				m_iDayNightStatus = DNS_DAY;
				Update_Weather();
				UTIL_BroadcastStuffText("spk %s\n", SOUND_DAY);

				timer_type timer(ioc);
				Set_Light(6, 0, 70.0f);
				for (int iLight = 6; iLight >= 0; --iLight)
				{
					Update_Environment(iLight);
					timer.expires_after(10s);
					co_await timer.async_wait(boost::asio::use_awaitable);
				}
				Set_Light(0, 0, 0.0f);

				// day to night
				auto wait_time = 90s;
				timer.expires_after(wait_time);
				co_await timer.async_wait(boost::asio::use_awaitable);

				// night pre start
				Set_Light(0, 6, 70.0f);
				for (int iLight = 0; iLight < 7; ++iLight)
				{
					Update_Environment(iLight);
					timer.expires_after(10s);
					co_await timer.async_wait(boost::asio::use_awaitable);
				}

				// night start
				m_iDayNightStatus = DNS_NIGHT;
				Update_Weather();
				UTIL_BroadcastStuffText("spk %s\n", SOUND_NIGHT);

				timer.expires_after(wait_time);
				co_await timer.async_wait(boost::asio::use_awaitable);
			}
			catch (const boost::system::system_error& e)
			{
				if (e.code() == boost::system::errc::operation_canceled)
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
		}

		void Set_SkyColor(float x = 1.0)
		{
			CVAR_SET_FLOAT("sv_skycolor_r", m_iSkyColor[0] * x);
			CVAR_SET_FLOAT("sv_skycolor_g", m_iSkyColor[1] * x);
			CVAR_SET_FLOAT("sv_skycolor_b", m_iSkyColor[2] * x);
		}

		void Set_EnvironmentFog(const char* szDensity = "0.0016", const char* szColor = "0 0 0")
		{
			mod->m_pFogEntity->m_fDensity = atof(szDensity);
			mod->m_pFogEntity->pev->rendercolor = { 0, 0, 0 };
			mod->m_pFogEntity->m_fBlendTime = 3.0f;
			mod->m_pFogEntity->UpdateClientMsg();
		}

		void Set_Light(int from, int to, float time)
		{
			std::string ls;
			if (time > 0)
				ls = std::to_string((int)time);
			if (from < to)
			{
				for (int i = from; i <= to; ++i)
				{
					ls.push_back(LIGHTSTYLE[i]);
				}
			}
			else
			{
				for (int i = from; i >= to; --i)
				{
					ls.push_back(LIGHTSTYLE[i]);
				}
			}
			LIGHT_STYLE(0, ls.c_str());
		}

		void Update_Weather()
		{
			auto iOld = m_iWeather;
			m_iWeather = (WeatherType_e)RANDOM_LONG(WT_CLOUDY, WT_MAX - 1);

			std::random_device rd;
			std::mt19937 gen(rd());
			const WeatherLevel_e N = (WeatherLevel_e)std::discrete_distribution<>(std::begin(WEATHER_LEVEL_WEIGHT), std::end(WEATHER_LEVEL_WEIGHT))(gen);
			m_iWeatherLevel = N;

			if (iOld != m_iWeather)
			{
				UTIL_BroadcastStuffText("stopsound\n");
				MESSAGE_BEGIN(MSG_BROADCAST, gmsgReceiveW);
				WRITE_BYTE(GetWeatherReceiveW(m_iWeather));
				MESSAGE_END();
			}
		}

		byte GetWeatherReceiveW(WeatherType_e iWeather)
		{
			switch (m_iWeather)
			{
			case WT_CLOUDY:
				return 0;
			case WT_SUNNY:
				return 0;
			case WT_RAINY:
				return 1;
			case WT_GUSTINESS:
				return 0;
			case WT_SNOWY:
				return 2;
			}
			return 0;
		}

		DayNightStatus_e GetDayNightStatus() const { return m_iDayNightStatus; }
	};

	CMod_ZombieShelter::CMod_ZombieShelter()
	{
		PRECACHE_SOUND(SOUND_DAY);
		PRECACHE_SOUND(SOUND_NIGHT);
		PRECACHE_SOUND(SOUND_BUILDING_SELECT);
		PRECACHE_SOUND(SOUND_BUILDING_SETTLE);
		PRECACHE_SOUND(SOUND_BUILDING_COMPLETE);
		PRECACHE_SOUND(SOUND_BUILDING_CRASH);
		PRECACHE_SOUND(SOUND_BUILDING_NO);
		PRECACHE_SOUND(SOUND_LEVELUP);
		PRECACHE_SOUND(SOUND_VITALITY_WARNING);

		round = std::make_unique<round_t>(this);
	}

	CMod_ZombieShelter::~CMod_ZombieShelter()
	{

	}

	void CMod_ZombieShelter::UpdateGameMode(CBasePlayer* pPlayer)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
		WRITE_BYTE(MOD_ZSH);
		WRITE_BYTE(0); // Reserved. (weapon restriction? )
		WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
		WRITE_BYTE(0); // Reserved. (MaxTime?)
		MESSAGE_END();
	}

	void CMod_ZombieShelter::RestartRound()
	{
		round = nullptr;
		round = std::make_unique<round_t>(this);
		return CMod_ZombieScenario::RestartRound();
	}

	void CMod_ZombieShelter::CheckMapConditions()
	{
		Base::CheckMapConditions();

		CBaseEntity *fog = nullptr;
		while ((fog = UTIL_FindEntityByClassname(fog, "env_fog")) != nullptr)
		{
			REMOVE_ENTITY(fog->edict());
		}

		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("env_fog"));
		if (!FNullEnt(pent))
		{
			CClientFog* pEntity = static_cast<CClientFog*>(CBaseEntity::Instance(pent));
			DispatchSpawn(pEntity->edict());
			m_pFogEntity = pEntity;
		}
	}

	void CMod_ZombieShelter::Think()
	{
		if (round)
			round->Think();

		TeamCheck();

		if (CheckGameOver())   // someone else quit the game already
			return;

		if (m_fTeamCount != invalid_time_point && m_fTeamCount <= gpGlobals->time)
		{
			if (m_iNumTerroristWins)
			{
				if (!CheckCanStartVoteMaps() && !m_iVoteMapStatus)
					GoToIntermission();
				else
					RestartRound();
			}
			else
				RestartRound();
		}

#ifdef XASH_DEDICATED
		// votemap things
		VoteMapThink();
#endif

		if (IsFreezePeriod())
		{
			static int iLastCountDown = -1;
			int iCountDown = TimeRemaining() / 1s;

			if (iCountDown > 0)
			{
				if (iCountDown != iLastCountDown)
				{
					iLastCountDown = iCountDown;
					if (iCountDown > 0 && iCountDown < 20)
					{
						UTIL_ClientPrintAll(HUD_PRINTCENTER, "#CSO_ZBS_StartCount", UTIL_dtos1(iCountDown));
					}

					if (iCountDown == 19)
					{
						WaitingSound();
					}
				}
			}
			else
			{
				RoundStart();
			}

			CheckFreezePeriodExpired();
		}

		CheckLevelInitialized();

		if (gpGlobals->time > m_tmNextPeriodicThink)
		{
			CheckRestartRound();
			m_tmNextPeriodicThink = gpGlobals->time + 1.0s;

			if (g_psv_accelerate->value != 5.0f)
			{
				CVAR_SET_FLOAT("sv_accelerate", 5.0);
			}

			if (g_psv_friction->value != 4.0f)
			{
				CVAR_SET_FLOAT("sv_friction", 4.0);
			}

			if (g_psv_stopspeed->value != 75.0f)
			{
				CVAR_SET_FLOAT("sv_stopspeed", 75.0);
			}

			m_iMaxRounds = (int)maxrounds.value;

			if (m_iMaxRounds < 0)
			{
				m_iMaxRounds = 0;
				CVAR_SET_FLOAT("mp_maxrounds", 0);
			}

			m_iMaxRoundsWon = (int)winlimit.value;

			if (m_iMaxRoundsWon < 0)
			{
				m_iMaxRoundsWon = 0;
				CVAR_SET_FLOAT("mp_winlimit", 0);
			}
		}

		if (FRoundStarted() && !m_bRoundTerminating)
		{
			// spawn zombie
			if (gpGlobals->time > m_flNextSpawnNPC)
			{
				MakeZombieNPC();
				m_flNextSpawnNPC = gpGlobals->time + 1.0s;
			}
		}

		/*if (TimeRemaining() <= 0s && !m_bRoundTerminating)
			HumanWin();*/
	}

	void CMod_ZombieShelter::InstallPlayerModStrategy(CBasePlayer* player)
	{
		std::unique_ptr<CPlayerModStrategy_ZSH> up = std::make_unique<CPlayerModStrategy_ZSH>(player, this);
		player->m_pModStrategy = std::move(up);
	}

	/* 增加团队储存的资源 */
	void CMod_ZombieShelter::AddResource(ResourceType_e type, int amount)
	{
		if (!round)
			return;

		round->m_iResource[type] = min(round->m_iResource[type] + amount, round->m_iMaxResource[type]);
	}

	/* 获取团队储存的最大资源 */
	int CMod_ZombieShelter::GetMaxResource(ResourceType_e type)
	{
		if (!round)
			return 0;

		return round->m_iMaxResource[type];
	}

	/* 设置团队储存的最大资源 */
	void CMod_ZombieShelter::SetMaxResource(ResourceType_e type, int amount)
	{
		if (!round)
			return;

		round->m_iMaxResource[type] = amount;
	}

	CPlayerModStrategy_ZSH::CPlayerModStrategy_ZSH(CBasePlayer* player, CMod_ZombieShelter* mp) : CPlayerModStrategy_Default(player), m_pModZSH(mp), m_iBuildProgress(8), m_iFixHealth(20), m_iRotate(0), m_iCollection(0), m_iSkillPoint(0)
	{
		m_hBuilding = nullptr;
		for (size_t i = 0; i < RT_MAX; i++)
		{
			m_iResource[i] = 0;
			m_iMaxResource[i] = 0;
			g_iReduceCost[i] = 0;
		}

		m_listenerAdjustDamage = mp->m_eventAdjustDamage.subscribe(
			[=](CBasePlayer* attacker, float& out)
			{
				//if (attacker == m_pPlayer)
					//out *= m_HumanLevel.GetAttackBonus();
			}
		);
		m_listenerMonsterKilled = mp->m_eventMonsterKilled.subscribe(
			[=](CMonster* victim, CBaseEntity* attacker)
			{
				if (attacker == m_pPlayer)
				{
					if (victim->m_iKillBonusFrags)
						m_pPlayer->AddPoints(victim->m_iKillBonusFrags, FALSE);
					if (victim->m_iKillBonusMoney)
						m_pPlayer->AddAccount(victim->m_iKillBonusMoney);

					MESSAGE_BEGIN(MSG_ONE, gmsgZBSTip, NULL, m_pPlayer->pev);
					WRITE_BYTE(ZBS_TIP_KILL);
					MESSAGE_END();
				}
			}
		);
	}

	bool CPlayerModStrategy_ZSH::CanPlayerBuy(bool display)
	{
		// is the player alive?
		if (m_pPlayer->pev->deadflag != DEAD_NO)
			return false;

		return true;
	}

	void CPlayerModStrategy_ZSH::ClientPutInServer()
	{
		m_flSpawnTime = invalid_time_point;
	}

	void CPlayerModStrategy_ZSH::OnSpawn()
	{
		m_pPlayer->SpawnProtection_Start(3.0s);
	}

	bool CPlayerModStrategy_ZSH::ClientCommand(const char* pcmd)
	{
		if (FStrEq(pcmd, "zsht_RequestBuild") && CMD_ARGC_() == 2)
		{
			for (auto&& pr : g_BuildingClassList)
			{
				if (FStrEq(std::get<1>(pr), CMD_ARGV_(1)))
				{
					CreateBuilding(std::get<0>(pr));
					return true;
				}
			}

			return true;
		}
		return CPlayerModStrategy_Default::ClientCommand(pcmd);
	}

	void CPlayerModStrategy_ZSH::OnInitHUD()
	{
		CPlayerModStrategy_Default::OnInitHUD();

		MESSAGE_BEGIN(MSG_ONE, gmsgReceiveW, NULL, m_pPlayer->pev);
		WRITE_BYTE(m_pModZSH->round->GetWeatherReceiveW(m_pModZSH->round->m_iWeather));
		MESSAGE_END();
	}

	void CPlayerModStrategy_ZSH::OnKilled(entvars_t* pKiller, entvars_t* pInflictor)
	{
		m_flSpawnTime = gpGlobals->time + m_pModZSH->round->m_iDays * 2s + 10s;
	}

	void CPlayerModStrategy_ZSH::OnThink()
	{
		if (!m_pModZSH->FRoundStarted() || m_pModZSH->m_bRoundTerminating)
			return;

		if ((m_pPlayer->pev->deadflag == DEAD_DEAD || m_pPlayer->pev->deadflag == DEAD_RESPAWNABLE) && m_pPlayer->m_iTeam == TEAM_CT)
		{
			if (m_flSpawnTime >= gpGlobals->time)
			{
				if (m_flSpawnTips <= gpGlobals->time)
				{
					m_flSpawnTips = gpGlobals->time + 1s;
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_ZBS_AutoRespawnWait", UTIL_dtos1((m_flSpawnTime - gpGlobals->time) / 1s));
				}
				return;
			}

			m_pPlayer->RoundRespawn();
			return;
		}

		if (m_pPlayer->pev->deadflag != DEAD_NO)
			return;

		if (m_hBuilding != nullptr)
		{
			TraceResult tr;
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			UTIL_TraceLine(m_pPlayer->EyePosition(), m_pPlayer->EyePosition() + gpGlobals->v_forward * 150, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			//tr.vecEndPos.z = m_pPlayer->pev->origin.z - (m_pPlayer->pev->flags & FL_DUCKING ? 16.0f : 36.0f) - m_hBuilding->m_pClass->GetMins().z;
			Vector to(tr.vecEndPos);
			to.z -= 9999.9f;
			UTIL_TraceLine(tr.vecEndPos, to, ignore_monsters, dont_ignore_glass, ENT(m_pPlayer->pev), &tr);
			tr.vecEndPos.z += -m_hBuilding->m_pClass->GetMins().z;
			UTIL_SetOrigin(m_hBuilding->pev, tr.vecEndPos);
		}
	}

	void CPlayerModStrategy_ZSH::GiveDefaultItems()
	{
		m_pPlayer->RemoveAllItems(FALSE);
		m_pPlayer->m_bHasPrimary = false;
		m_pPlayer->GiveNamedItem("weapon_knife");
	}

	void CPlayerModStrategy_ZSH::CmdStart(struct usercmd_s* cmd, unsigned int random_seed)
	{
		if (m_hBuilding == nullptr)
			return;

		if (m_pPlayer->pev->deadflag != DEAD_NO)
		{
			m_hBuilding.Remove();
			m_hBuilding = nullptr;
			return;
		}

		auto& bitsCurButton = cmd->buttons;
		auto& bitsOldButton = m_pPlayer->pev->oldbuttons;

		if (bitsCurButton & IN_ATTACK)
		{
			if (!m_hBuilding->Settle())
			{
				CLIENT_COMMAND(m_pPlayer->edict(), "spk %s\n", SOUND_BUILDING_NO);
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_ZSHT_ERROR_NOT_ENOUGH_SPACE");
				return;
			}

			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_STATIC, SOUND_BUILDING_SETTLE, VOL_NORM, ATTN_NORM);
			UTIL_ClientPrintAll(HUD_PRINTTALK, "#CSO_zsht_Build_Message", STRING(m_pPlayer->pev->netname), m_hBuilding->m_pClass->GetName());
			m_hBuilding = nullptr;
		}
		else if (bitsCurButton & IN_ATTACK2)
		{
			m_hBuilding.Remove();
			m_hBuilding = nullptr;
		}
		else if (bitsCurButton & IN_RELOAD && ~bitsOldButton & IN_RELOAD)
		{
			m_hBuilding->pev->angles.y = (++m_iRotate % 4) * 90.0f;
		}
	}


	void CPlayerModStrategy_ZSH::UpdateStatusBar(CBaseEntity* pEntity, char* sbuf0, int* newSBarState)
	{
		if (!FClassnameIs(pEntity->pev, "zsh_building"))
			return;

		Q_strcpy(sbuf0, "1 %c1%i3(%i4%%)");

		newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_WALL;
		newSBarState[SBAR_ID_TARGETHEALTH] = pEntity->pev->health;
		newSBarState[SBAR_ID_TARGETHEALTH_PERCENT] = (int)((pEntity->pev->health / pEntity->pev->max_health) * 100);

		m_pPlayer->m_flStatusBarDisappearDelay = gpGlobals->time + 2.0s;
	}

	CZSHBuilding* CPlayerModStrategy_ZSH::CreateBuilding(const ZSHBuildingClass iBuilding)
	{
		CZSHBuilding* pBuilding = CreateClassPtr<CZSHBuilding>();

		if (!pBuilding)
			return nullptr;

		if (m_hBuilding != nullptr)
		{
			m_hBuilding.Remove();
			m_hBuilding = nullptr;
		}

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_STATIC, SOUND_BUILDING_SELECT, VOL_NORM, ATTN_NORM);
		m_hBuilding = pBuilding;
		pBuilding->Create(m_pPlayer, iBuilding, BuildingClassFactory(iBuilding, pBuilding));
		pBuilding->pev->angles.y = (m_iRotate % 4) * 90.0f;
		return pBuilding;
	}

	void CPlayerModStrategy_ZSH::AddResource(ResourceType_e type, int amount)
	{
	}

}