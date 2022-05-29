#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"
#endif

namespace sv {
#ifndef CLIENT_DLL
	class CM95TigerNet : public CBaseEntity
	{
	public:
		CM95TigerNet()
		{
			m_flLockTime = 1.5s;
			m_flRemoveTime = invalid_time_point;
		}

		void Spawn();
		void Precache();
		void EXPORT OnTouch(CBaseEntity* pOther);
		void EXPORT FlyThink();
		void Remove();
		void SetLockTime(duration_t flTime)
		{
			m_flLockTime = flTime;
		}

		duration_t m_flLockTime;
		time_point_t m_flRemoveTime;
	};
#endif
}
