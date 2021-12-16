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
			static constexpr auto Sound_Skill = "zombi/z4/normal/normal_skill.wav";

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
				m_pPlayer->pev->gravity = 0.7f;
				m_pPlayer->ResetMaxSpeed();
			}
		};
	}
}