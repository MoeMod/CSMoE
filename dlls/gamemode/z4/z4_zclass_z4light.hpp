#pragma once

namespace sv {
	namespace z4 {
		struct zclass_z4light {
			static constexpr auto Name = "暗夜魅影";
			static constexpr auto Model = "z4_light";
			static constexpr auto V_Model = "models/v_knife_z4light.mdl";
			static constexpr zclass_sound_s Sound = {
				"zombi/z4/light/light_death.wav",
				"zombi/z4/light/light_death.wav",
				"zombi/z4/light/light_gut_flinch1.wav",
				"zombi/z4/light/light_gut_flinch2.wav",
				"",
				"",
				"zombi/z4/light/light_stun.wav",
				"zombi/z4/light/light_skill_start.wav",
				"zombi/z4/light/light_dash.wav"
			};
			static constexpr zclass_ability_s Ability = {
					285.0f, 0.7f, 1.5f, 1.1f, 295.0f, 0.7f, 1.32f, 2.4f
			};
			static constexpr zclass_skill_s Skill = {
					"飞跃", "潜行", 5.0f, 15.0f, 0.0f, 20.0f, 15.0s, 0.25s, 60, 3.0s
			};

			CBasePlayer *const m_pPlayer;
			bool m_bIsUsingSkill = false;
			int m_iInvisibleLevel = 0;

			static constexpr int MAX_RENDER = 14;
			static constexpr int cfg_iRenderAmt[MAX_RENDER] = { 245, 230, 215, 200, 185, 170, 155, 140, 125, 110, 95, 80, 65, 50 };

			explicit zclass_z4light(CBasePlayer *player) : m_pPlayer(player)
			{
				m_pPlayer->m_ZombieClass = "z4light";

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
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_STATIC, Sound.szSkill2, VOL_NORM, ATTN_NORM);

				m_pPlayer->pev->renderfx = kRenderFxNone;
				m_pPlayer->pev->rendercolor = {255, 255, 255};
				m_pPlayer->pev->rendermode = kRenderTransAlpha;
				m_pPlayer->pev->renderamt = 255;

				m_iInvisibleLevel = 1;
			}

			void OnDashCheck()
			{
				m_iInvisibleLevel = std::min(m_iInvisibleLevel + 1, MAX_RENDER - 1);

				m_pPlayer->pev->renderfx = kRenderFxNone;
				m_pPlayer->pev->rendercolor = {255, 255, 255};
				m_pPlayer->pev->rendermode = kRenderTransAlpha;
				m_pPlayer->pev->renderamt = cfg_iRenderAmt[m_iInvisibleLevel];
			}

			void OnDashEnd()
			{
				m_iInvisibleLevel = 0;

				m_pPlayer->pev->renderfx = kRenderFxNone;
				m_pPlayer->pev->rendercolor = {255, 255, 255};
				m_pPlayer->pev->rendermode = kRenderNormal;
				m_pPlayer->pev->renderamt = 16;
			}

			Vector get_position(float f, float r, float u) const // copied from amxx
			{
				UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
				Vector vecSrc = m_pPlayer->GetGunPosition();
				return vecSrc + gpGlobals->v_forward * f + gpGlobals->v_right * r + gpGlobals->v_up * u;
			}

			Vector Get_SpeedVector(Vector origin1, Vector origin2, float speed)
			{
				Vector new_velocity = origin2 - origin1;
				new_velocity[0] *= 2;
				new_velocity[1] *= 2;
				new_velocity[2] /= 2;
				return new_velocity.Normalize() * speed;
			}

			bool OnSkillActivate()
			{
				if(m_pPlayer->pev->flags & FL_DUCKING)
					return false;
				if(!(m_pPlayer->pev->flags & FL_ONGROUND))
					return false;
				if(m_bIsUsingSkill)
					return false;

				auto vecOrigin = m_pPlayer->pev->origin;
				auto vecEnd = get_position(180, 0, 650);
				auto vecVelocity = Get_SpeedVector(vecOrigin, vecEnd, 1000);
				m_pPlayer->pev->velocity = vecVelocity;

				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_SKILL_START, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck;
				}
				m_pPlayer->m_flNextAttack = 1.0s;
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, Sound.szSkill1, VOL_NORM, ATTN_NORM);

				m_pPlayer->m_iCustomSequence = m_pPlayer->LookupSequence("skill_start");
				m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.4s;
				m_bIsUsingSkill = true;
				return true;
			}

			void OnThink()
			{
				if(m_bIsUsingSkill && (m_pPlayer->pev->flags & FL_ONGROUND))
				{
					if(m_pPlayer->pev->velocity[2] == 0.0)
					{
						if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
						{
							pWeapon->SendWeaponAnim(ANIM_SKILL_END, TRUE);
							pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck;
						}
						m_bIsUsingSkill = false;
						m_pPlayer->m_iCustomSequence = m_pPlayer->LookupSequence("idle1");
						m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.4s;
					}
				}
			}

			void CmdStart(usercmd_t *cmd, unsigned int random_seed) {}

		};
	}
}