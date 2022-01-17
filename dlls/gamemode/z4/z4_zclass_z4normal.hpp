#pragma once

namespace sv {
	namespace z4 {
		struct zclass_z4normal {
			static constexpr auto Name = "残暴镰魔";
			static constexpr auto Model = "z4_normal";
			static constexpr auto V_Model = "models/v_knife_z4normal.mdl";
			static constexpr zclass_sound_s Sound = {
				"zombi/z4/normal/normal_death.wav",
				"zombi/z4/normal/normal_death.wav",
				"zombi/z4/normal/normal_hurt1.wav",
				"zombi/z4/normal/normal_hurt2.wav",
				"zombi/z4/normal/normal_dash1.wav",
				"zombi/z4/normal/normal_dash2.wav",
				"zombi/z4/normal/normal_stun.wav",
				"zombi/z4/normal/normal_skill.wav"
			};
			static constexpr zclass_ability_s Ability = {
					265.0f, 0.8f, 0.7f, 0.8f, 360.0f, 0.8f, 0.5f, 0.8f
			};
			static constexpr zclass_skill_s Skill = {
					"攀爬", "暴走", 10.0, 25.0, 0.0, 20.0, 15.0s, 0.5s, 11, 0.5s
			};

			CBasePlayer *const m_pPlayer;

			explicit zclass_z4normal(CBasePlayer *player) : m_pPlayer(player)
			{
				m_pPlayer->m_ZombieClass = "z4normal";

				SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", Model);

				static char szModelPath[64];
				Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", Model, Model);
				m_pPlayer->SetNewPlayerModel(szModelPath);

				// set zombie property
				m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
				m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
				m_pPlayer->pev->armorvalue = 200;
				m_pPlayer->pev->gravity = Ability.flGravity;
				m_pPlayer->ResetMaxSpeed();
			}

			void OnDashStart()
			{
				// nothing
			}

			void OnDashCheck()
			{
				// nothing
			}

			void OnDashEnd()
			{
				// nothing
			}

			time_point_t m_flTimeSkillEnd = invalid_time_point;

			void OnSkillActivate()
			{
				if(m_pPlayer->pev->flags & FL_DUCKING)
					return;

				UTIL_MakeVectors(m_pPlayer->pev->v_angle);
				Vector vecSrc = m_pPlayer->GetGunPosition();
				Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

				TraceResult tr;
				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
				if(tr.flFraction >= 1.0f)
					return;

				m_pPlayer->pev->velocity[2] = 300.0f;

				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
				if (pWeapon)
				{
					pWeapon->SendWeaponAnim(ANIM_SKILL, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck;
				}
				m_pPlayer->m_iCustomSequence = m_pPlayer->LookupSequence("skill_start");
				m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.4s;

				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, Sound.szSkill1, VOL_NORM, ATTN_NORM);

				m_flTimeSkillEnd = gpGlobals->time + 0.4s;
			}

			void OnThink()
			{
				if(m_flTimeSkillEnd != invalid_time_point && gpGlobals->time > m_flTimeSkillEnd)
				{
					m_flTimeSkillEnd = invalid_time_point;

					CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
					if (pWeapon)
					{
						pWeapon->SendWeaponAnim(ANIM_SKILL_END, TRUE);
						pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck;
					}

					m_pPlayer->m_iCustomSequence = m_pPlayer->LookupSequence("idle1");
					m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.4s;
				}
			}
		};
	}
}