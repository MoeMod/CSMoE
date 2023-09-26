#pragma once

namespace sv {
	namespace z4 {
		struct zclass_z4hide {
			static constexpr auto Name = "幽暗领主";
			static constexpr auto Model = "z4_hide";
			static constexpr auto V_Model = "models/v_knife_z4hide.mdl";
			static constexpr zclass_sound_s Sound = {
					"zombi/z4/hidden/hide_death.wav",
					"zombi/z4/hidden/hide_death.wav",
					"zombi/z4/hidden/hide_hurt1.wav",
					"zombi/z4/hidden/hide_hurt2.wav",
					"zombi/z4/hidden/hide_dash1.wav",
					"zombi/z4/hidden/hide_dash2.wav",
					"zombi/z4/hidden/hide_stun.wav",
					"zombi/z4/hidden/hide_skill_start.wav"
			};
			static constexpr zclass_ability_s Ability = {
					270.0f, 0.75f, 0.6f, 0.65f, 310.0f, 0.75f, 0.25f, 0.325f
			};
			static constexpr zclass_skill_s Skill = {
					"制空", "冲锋", 10.0f, 25.0f, 0.0f, 20.0f, 15.0s, 0.5s, 25, 0.5s
			};

			CBasePlayer *const m_pPlayer;
			bool m_bIsUsingSkill = false;

			explicit zclass_z4hide(CBasePlayer *player) : m_pPlayer(player)
			{
				m_pPlayer->m_iZombieClass = ZOMBIE_CLASS_Z4HIDDEN;

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

			bool CheckCanDash()
			{
				if (m_pPlayer->pev->flags & FL_DUCKING)
					return false;

				return true;
			}

			void OnDashStart()
			{
				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_DASH_START, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck + 0.1s;
				}
			}

			void OnDashCheck()
			{
				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_DASH_LOOP, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck + 0.1s;
				}
			}

			void OnDashEnd()
			{
				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_DASH_END, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck + 0.1s;
				}
			}

			Vector get_position(float f, float r, float u) const // copied from amxx
			{
				UTIL_MakeVectors(m_pPlayer->pev->v_angle);
				Vector vecSrc = m_pPlayer->GetGunPosition();
				return vecSrc + gpGlobals->v_forward * f + gpGlobals->v_right * r + gpGlobals->v_up * u;
			}

			Vector Get_SpeedVector(Vector origin1, Vector origin2, float speed)
			{
				Vector new_velocity = (origin2 - origin1).Normalize() * speed;
				new_velocity[2] *= 0.5f;
				return new_velocity;
			}

			bool OnSkillActivate()
			{
				if(m_pPlayer->pev->flags & FL_DUCKING)
					return false;

				if((m_pPlayer->pev->flags & FL_ONGROUND))
				{
					auto vecOrigin = m_pPlayer->pev->origin;
					auto vecEnd = get_position(30, 0, 250);
					auto vecVelocity = Get_SpeedVector(vecOrigin, vecEnd, 400);
					vecVelocity[2] = 550;
					m_pPlayer->pev->velocity = vecVelocity;

					if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
					{
						pWeapon->SendWeaponAnim(ANIM_SKILL_START, TRUE);
						pWeapon->m_flTimeWeaponIdle = 0.5s;
					}
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, Sound.szSkill1, VOL_NORM, ATTN_NORM);
				}
				else
				{
					auto vecOrigin = m_pPlayer->pev->origin;
					auto vecEnd = get_position(250, 0, 60);
					auto vecVelocity = Get_SpeedVector(vecOrigin, vecEnd, 400);
					vecVelocity[2] = 200;
					m_pPlayer->pev->velocity = vecVelocity;

					if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
					{
						//pWeapon->SendWeaponAnim(ANIM_SKILL_START, TRUE);
						pWeapon->m_flTimeWeaponIdle = 0.5s;
					}
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, Sound.szSkill1, VOL_NORM, ATTN_NORM);
				}

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
							pWeapon->m_flTimeWeaponIdle = 0.3s;
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