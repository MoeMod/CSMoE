#pragma once

#include <variant>
#include <type_traits>
#include "meta/StructBreak.hpp"

namespace sv {
	namespace z4 {
		struct zclass_sound_s {
			const char *szSoundDeath1;
			const char *szSoundDeath2;
			const char *szSoundPain1;
			const char *szSoundPain2;
			const char *szSoundDash1;
			const char *szSoundDash2;
			const char *szSoundStun;
			const char *szSkill1;
			const char *szSkill2;
		};

		struct zclass_ability_s {
			float flSpeed;
			float flGravity;
			float flKnockback;
			float flDefense;
			float flDashSpeed;
			float flDashGravity;
			float flDashKnockback;
			float flDashDefense;
		};

		struct zclass_skill_s {
			const char *szNameSkill;
			const char *szNameDash;
			float flRegenDusk, flRegenDark, flRegenDawn, flExtraHealthPercent;
			duration_t flDashLasting, flDashTimeCheck;
			int iSkillConsume;
			duration_t flSkillTimeReset;
		};

		enum z4_knife_e
		{
			ANIM_IDLE1,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_DRAW,
			ANIM_STUN_START,
			ANIM_STUN_END,
			ANIM_SKILL_START,
			ANIM_SKILL_END,
			ANIM_SKILL,
			ANIM_DASH_START,
			ANIM_DASH_LOOP,
			ANIM_DASH_END,
		};
	}
}

#include "z4_zclass_z4normal.hpp"
#include "z4_zclass_z4light.hpp"
#include "z4_zclass_z4heavy.hpp"
#include "z4_zclass_z4hide.hpp"
#include "z4_zclass_z4humpback.hpp"

namespace sv {
	namespace z4 {
		using zclass_variant_t = std::variant<
	        zclass_z4normal,
			zclass_z4light,
			zclass_z4heavy,
			zclass_z4hide,
			zclass_z4humpback
		>;

		template<class ZC> auto Z4_ZClass_Precache_Single_Custom(ZC *) -> decltype(ZC::Precache())
		{
			return ZC::Precache();
		}
		template<class ZC> void Z4_ZClass_Precache_Single_Custom(...) {}

		template<class ZC> void Z4_ZClass_Precache_Single()
		{
			char buffer[MAX_PATH];
			sprintf(buffer, "models/player/%s/%s.mdl", ZC::Model, ZC::Model);
			PRECACHE_MODEL(buffer);

			PRECACHE_MODEL(ZC::V_Model);

			StructApply(ZC::Sound, [](auto...SoundList){
				(..., (SoundList && PRECACHE_SOUND(SoundList)));
			});

			Z4_ZClass_Precache_Single_Custom<ZC>(nullptr);
		}

		template<class...ZC>
		void Z4_ZClass_Precache_All(std::variant<ZC...> *)
		{
			(..., Z4_ZClass_Precache_Single<ZC>());
		}
		inline void Z4_ZClass_Precache_All()
		{
			return Z4_ZClass_Precache_All((zclass_variant_t *)nullptr);
		}

		template<class...ZC> zclass_variant_t Z4_MakeRandomZombie(std::variant<ZC...> *, CBasePlayer *player)
		{
			auto Index = RANDOM_LONG(0, sizeof...(ZC) - 1);
			constexpr zclass_variant_t(*pfn[sizeof...(ZC)])(CBasePlayer *player) = { [](CBasePlayer *player) -> zclass_variant_t { return ZC(player); }... };
			return pfn[Index](player);
		}
		zclass_variant_t Z4_MakeRandomZombie(CBasePlayer *player)
		{
			return Z4_MakeRandomZombie((zclass_variant_t *)nullptr, player);
		}
	}
}