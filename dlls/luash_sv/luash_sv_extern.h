/*
luash_sv_extern.h
Copyright (C) 2022 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef EXTERN_TEMPLATE
#define EXTERN_TEMPLATE extern
#endif

#ifndef EXTERN_TEMPLATE_DEF
#define EXTERN_TEMPLATE_DEF(T) EXTERN_TEMPLATE template const ITypeInterface* CreateTypeInterface<T>();
#endif

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	class CBaseEntity;
	class CBasePlayer;
	class CBasePlayerItem;
	class CBasePlayerWeapon;
}

namespace luash {

	EXTERN_TEMPLATE_DEF(edict_t)
	EXTERN_TEMPLATE_DEF(entvars_t)

#ifndef CLIENT_DLL
	EXTERN_TEMPLATE_DEF(sv::CBaseEntity)
	EXTERN_TEMPLATE_DEF(sv::CBasePlayer)
	EXTERN_TEMPLATE_DEF(sv::CBasePlayerItem)
	EXTERN_TEMPLATE_DEF(sv::CBasePlayerWeapon)
#else
	EXTERN_TEMPLATE_DEF(cl::CBaseEntity)
	EXTERN_TEMPLATE_DEF(cl::CBasePlayer)
	EXTERN_TEMPLATE_DEF(cl::CBasePlayerItem)
	EXTERN_TEMPLATE_DEF(cl::CBasePlayerWeapon)
#endif
	EXTERN_TEMPLATE_DEF(TraceResult)
	EXTERN_TEMPLATE_DEF(KeyValueData)
	EXTERN_TEMPLATE_DEF(Vector)

}

#undef EXTERN_TEMPLATE_DEF
#undef EXTERN_TEMPLATE