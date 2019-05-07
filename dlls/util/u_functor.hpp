/*
u_functor.hpp - some functors wrapping UTIL_* with Modern C++
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_U_FUNCTOR_HPP
#define PROJECT_U_FUNCTOR_HPP

namespace moe
{
	// !!! BOT's classname is not "player", so UTIL_FindEntityByClassname(p, "player") CANNOT enum players
	struct Enumer_Player
	{
		CBasePlayer *operator()(CBasePlayer *pCurrent) const
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

		explicit Enumer_ClassName(const char *classname) : m_classname(classname) {}

		EntityType *operator()(EntityType *pCurrent) const
		{
			return static_cast<EntityType *>(UTIL_FindEntityByClassname(pCurrent, m_classname));
		}
	};
}

#endif //PROJECT_U_FUNCTOR_HPP
