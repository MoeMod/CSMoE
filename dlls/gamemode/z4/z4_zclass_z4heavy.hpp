#pragma once

namespace sv {
	namespace z4 {
		struct zclass_z4heavy {
			static constexpr auto Name = "重锤血手";
			static constexpr auto Model = "z4_heavy";
			static constexpr auto V_Model = "models/v_knife_z4heavy.mdl";
			static constexpr zclass_sound_s Sound = {
				"zombi/z4/heavy/z4heavy_death.wav",
					"zombi/z4/heavy/z4heavy_death.wav",
					"zombi/z4/heavy/heavy_hurt1.wav",
					"zombi/z4/heavy/heavy_hurt2.wav",
					"zombi/z4/heavy/z4heavy_dash1.wav",
					"zombi/z4/heavy/z4heavy_dash2.wav",
					"zombi/z4/heavy/z4heavy_stun.wav",
					"zombi/z4/heavy/z4heavy_skill_start.wav",
				"zombi/z4/heavy/z4heavy_skill_end.wav"
			};
			static constexpr zclass_ability_s Ability = {
					255.0f, 0.85f, 0.4f, 0.55f, 175.0f, 0.85f, 0.1f, 0.165f
			};
			static constexpr zclass_skill_s Skill = {
					"撼地", "防护", 10.0f, 25.0f, 0.0f, 20.0f, 15.0s, 0.5s, 25, 1.0s
			};

			CBasePlayer *const m_pPlayer;
			bool m_bIsUsingSkill = false;

			static inline int s_iSprShockWave = 0;

			static void Precache()
			{
				s_iSprShockWave = PRECACHE_MODEL("sprites/shockwave.spr");
			}

			explicit zclass_z4heavy(CBasePlayer *player) : m_pPlayer(player)
			{
				m_pPlayer->m_ZombieClass = "z4heavy";

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
				auto vecEnd = get_position(30, 0, 200);
				auto vecVelocity = Get_SpeedVector(vecOrigin, vecEnd, 500);
				m_pPlayer->pev->velocity = vecVelocity;

				if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
				{
					pWeapon->SendWeaponAnim(ANIM_SKILL_START, TRUE);
					pWeapon->m_flTimeWeaponIdle = 5.0s;
				}
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
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, Sound.szSkill2, VOL_NORM, ATTN_NORM);

						Skill_Out();
					}
				}
			}

			void Skill_Out()
			{
				auto vecOrigin = m_pPlayer->pev->origin;
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecOrigin);
				WRITE_BYTE(TE_BEAMCYLINDER);
				WRITE_COORD(vecOrigin[0]);
				WRITE_COORD(vecOrigin[1]);
				WRITE_COORD(vecOrigin[2]);
				WRITE_COORD(vecOrigin[0]);
				WRITE_COORD(vecOrigin[1]);
				WRITE_COORD(vecOrigin[2] + 300.0);
				WRITE_SHORT(s_iSprShockWave);
				WRITE_BYTE(0); // Start Frame
				WRITE_BYTE(20); // Framerate
				WRITE_BYTE(4); // Live Time
				WRITE_BYTE(16); // Width
				WRITE_BYTE(10); // Noise
				WRITE_BYTE(255); // R
				WRITE_BYTE(85); // G
				WRITE_BYTE(85); // B
				WRITE_BYTE(255); // Bright
				WRITE_BYTE(9); // Speed
				MESSAGE_END();

				CBaseEntity* pEntity = NULL;
				while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, 250)) != NULL)
				{
					if (pEntity->pev->takedamage != DAMAGE_NO)
					{
						if (pEntity->IsPlayer())
						{
							CBasePlayer* player = static_cast<CBasePlayer*>(pEntity);

							if (player == m_pPlayer)
								continue;

							if (player->IsAlive() && !player->m_bIsZombie)
							{
								auto vecNewVelocity = Get_SpeedVector(m_pPlayer->pev->origin, player->pev->origin, 250);
								vecNewVelocity[2] = 175;
								player->pev->velocity += vecNewVelocity;

								MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, player->edict());
								WRITE_SHORT((1<<12) * 4);
								WRITE_SHORT((1<<12) * 5);
								WRITE_SHORT((1<<12) * 10);
								MESSAGE_END();

								Vector vecPunch = { RANDOM_FLOAT(-35.0, 35.0), RANDOM_FLOAT(-35.0, 35.0), RANDOM_FLOAT(-35.0, 35.0) };
								player->pev->punchangle = vecPunch;
							}
						}
					}
				}
			}

			void CmdStart(usercmd_t *cmd, unsigned int random_seed) {}

		};
	}
}