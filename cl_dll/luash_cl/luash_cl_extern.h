/*
luash_cl_extern.h
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

namespace luash {

	EXTERN_TEMPLATE_DEF(client_textmessage_t)
	EXTERN_TEMPLATE_DEF(hud_player_info_t)
	EXTERN_TEMPLATE_DEF(SCREENINFO)
	EXTERN_TEMPLATE_DEF(screenfade_t)
	EXTERN_TEMPLATE_DEF(con_nprint_t)
	EXTERN_TEMPLATE_DEF(cl_entity_t)
	EXTERN_TEMPLATE_DEF(cvar_t)
	EXTERN_TEMPLATE_DEF(event_args_t)
	EXTERN_TEMPLATE_DEF(pmtrace_t)
	EXTERN_TEMPLATE_DEF(edict_t)
	EXTERN_TEMPLATE_DEF(client_sprite_t)
	EXTERN_TEMPLATE_DEF(model_t)
	EXTERN_TEMPLATE_DEF(const model_t)
	EXTERN_TEMPLATE_DEF(POINT)
	EXTERN_TEMPLATE_DEF(const POINT)
	EXTERN_TEMPLATE_DEF(wrect_t)
	EXTERN_TEMPLATE_DEF(const wrect_t)

	EXTERN_TEMPLATE_DEF(void)
	EXTERN_TEMPLATE_DEF(const void)
	EXTERN_TEMPLATE_DEF(int)
	EXTERN_TEMPLATE_DEF(const int)
	EXTERN_TEMPLATE_DEF(char)
	EXTERN_TEMPLATE_DEF(const char)
	EXTERN_TEMPLATE_DEF(float)
	EXTERN_TEMPLATE_DEF(const float)
	EXTERN_TEMPLATE_DEF(cl::Vector)
	EXTERN_TEMPLATE_DEF(const cl::Vector)

}

#undef EXTERN_TEMPLATE_DEF
#undef EXTERN_TEMPLATE