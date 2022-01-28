#pragma once

namespace sv {
	namespace z4 {
		struct zclass_z4humpback {
			static constexpr auto Name = "厄运之轮";
			static constexpr auto Model = "z4_humpback";
			static constexpr auto V_Model = "models/v_knife_z4humpback.mdl";
			static constexpr zclass_sound_s Sound = {
					"zombi/z4/hump_back/death.wav",
					"zombi/z4/hump_back/death.wav",
					"zombi/z4/hump_back/hurt1.wav",
					"zombi/z4/hump_back/hurt2.wav",
					"zombi/z4/hump_back/dash_loop.wav",
					"zombi/z4/hump_back/dash_loop.wav",
					"zombi/z4/hump_back/stun.wav",
					"zombi/z4/hump_back/skill_start.wav"
			};
			static constexpr zclass_ability_s Ability = {
					260.0f, 0.75f, 0.85f, 0.65f, 975.0f, 0.8f, 0.15f, 0.45f
			};
			static constexpr zclass_skill_s Skill = {
					"飞跃", "战轮", 8.0f, 20.0f, 0.0f, 20.0f, 15.0s, 0.25s, 45, 1.0s
			};

			CBasePlayer *const m_pPlayer;
			bool m_bIsUsingSkill = false;
			bool m_bIsUsingDash = false;

			static inline int s_iSprDust[2] = {};

			static void Precache()
			{
				s_iSprDust[0] = PRECACHE_MODEL("sprites/z4/dust.spr");
				s_iSprDust[1] = PRECACHE_MODEL("sprites/z4/dust2.spr");
			}

			explicit zclass_z4humpback(CBasePlayer *player) : m_pPlayer(player)
			{
				m_pPlayer->m_ZombieClass = "z4humpback";

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
				return true;
			}

			void OnDashStart()
			{
				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_DASH_START, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck;
				}
				m_pPlayer->pev->bInDuck = 1;
				CLIENT_COMMAND(m_pPlayer->edict(), "+duck");
				CLIENT_COMMAND(m_pPlayer->edict(), "cl_forwardspeed 1000");
				m_bIsUsingDash = true;
			}

			void OnDashCheck()
			{
				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_DASH_LOOP, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck + 0.1s;
				}
				m_pPlayer->m_flVelocityModifier = 1.0;
				m_pPlayer->m_flNextAttack = 0.5s;
				Make_Smoke(1);
			}

			void OnDashEnd()
			{
				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_DASH_END, TRUE);
					pWeapon->m_flTimeWeaponIdle = Skill.flDashTimeCheck + 0.1s;
				}

				m_pPlayer->pev->bInDuck = 0;
				CLIENT_COMMAND(m_pPlayer->edict(), "-duck");
				CLIENT_COMMAND(m_pPlayer->edict(), "cl_forwardspeed 400");
				m_bIsUsingDash = false;
			}

			Vector get_position(float f, float r, float u) const // copied from amxx
			{
				UTIL_MakeVectors(m_pPlayer->pev->v_angle);
				Vector vecSrc = m_pPlayer->GetGunPosition();
				return vecSrc + gpGlobals->v_forward * f + gpGlobals->v_right * r + gpGlobals->v_up * u;
			}

			Vector Get_SpeedVector(Vector origin1, Vector origin2, float speed)
			{
				Vector new_velocity = origin2 - origin1;
				float val = speed / new_velocity.LengthReverse();
				new_velocity[0] *= (val * 2);
				new_velocity[1] *= (val * 2);
				new_velocity[2] *= (val / 2);
				return new_velocity;
			}

			bool OnSkillActivate()
			{
				if(m_pPlayer->pev->flags & FL_DUCKING)
					return false;
				if(!(m_pPlayer->pev->flags & FL_ONGROUND))
					return false;
				if(m_bIsUsingSkill)
					return false;

				auto vecStart = m_pPlayer->GetGunPosition();
				auto vecEnd = get_position(512, 0, 320);

				auto vecVelocity = Get_SpeedVector(vecStart, vecEnd, 1024.0);
				m_pPlayer->pev->velocity = vecVelocity;

				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_SKILL_START, TRUE);
					pWeapon->m_flTimeWeaponIdle = 0.5s;
				}
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, Sound.szSkill1, VOL_NORM, ATTN_NORM);

				m_pPlayer->m_iCustomSequence = m_pPlayer->LookupSequence("skill_start");
				m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.4s;
				m_bIsUsingSkill = true;

				Make_Smoke(0);
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
				if(m_bIsUsingDash)
				{
					m_pPlayer->pev->bInDuck = 1;
					CLIENT_COMMAND(m_pPlayer->edict(), "+duck");
					SET_CLIENT_MAXSPEED(m_pPlayer->edict(), Ability.flDashSpeed);
					m_pPlayer->pev->maxspeed = Ability.flDashSpeed;
				}
			}

			void Make_Smoke(int iSpr)
			{
				auto vecOrigin = m_pPlayer->pev->origin;
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecOrigin);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(vecOrigin[0]);
				WRITE_COORD(vecOrigin[1]);
				WRITE_COORD(vecOrigin[2]);
				WRITE_SHORT(s_iSprDust[iSpr]);
				WRITE_BYTE(RANDOM_LONG(4, 7));
				WRITE_BYTE(30); // Framerate
				WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS|TE_EXPLFLAG_NOSOUND|TE_EXPLFLAG_NOPARTICLES);
				MESSAGE_END();
			}

			void CmdStart(usercmd_t *cmd, unsigned int random_seed)
			{
				if(m_bIsUsingDash)
				{
					cmd->buttons |= IN_DUCK;
				}
			}

		};
	}
}