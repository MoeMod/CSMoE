//
// Created by 小白白 on 2019-02-09.
//

#ifndef PROJECT_U_FUNCTOR_HPP
#define PROJECT_U_FUNCTOR_HPP

namespace moe
{
	// !!! BOT's classname is not "player", so UTIL_FindEntityByClassname(p, "player") CANNOT enum players
	struct Enumer_Player
	{
		CBasePlayer *operator()(CBasePlayer *pCurrent)
		{
			for(int iCurrentIndex = pCurrent ? pCurrent->entindex() + 1 : 1; iCurrentIndex <= gpGlobals->maxClients; ++iCurrentIndex)
			{
				CBaseEntity *p = UTIL_PlayerByIndex(iCurrentIndex);
				if(p)
					return static_cast<CBasePlayer *>(p);
			}
			return nullptr;
		}
	};

	template<class EntityType = CBaseEntity>
	struct Enumer_ClassName
	{
		const char * const m_classname;
		Enumer_ClassName(const char *classname) : m_classname(classname) {}

		EntityType *operator()(EntityType *pCurrent)
		{
			return static_cast<EntityType *>(UTIL_FindEntityByClassname(pCurrent, m_classname));
		}
	};
}

#endif //PROJECT_U_FUNCTOR_HPP
