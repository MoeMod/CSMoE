// no pragma once !!!
//#pragma once

#ifndef CLASS_BEGIN
#define CLASS_BEGIN(ClassName, BaseClassName)
#endif
#ifndef ADD_VIRTUAL_FUNCTION
#define ADD_VIRTUAL_FUNCTION(FunctionName, ...)
#endif
#ifndef ADD_OVERRIDE_VIRTUAL_FUNCTION
#define ADD_OVERRIDE_VIRTUAL_FUNCTION(FunctionName, ...)
#endif
#ifndef ADD_MEMBER_FUNCTION
#define ADD_MEMBER_FUNCTION(FunctionName, ...)
#endif
#ifndef ADD_STATIC_MEMBER_FUNCTION
#define ADD_STATIC_MEMBER_FUNCTION(FunctionName, ...)
#endif
#ifndef ADD_MEMBER_DATA
#define ADD_MEMBER_DATA(MemberName)
#endif
#ifndef ADD_STATIC_MEMBER_DATA
#define ADD_STATIC_MEMBER_DATA(MemberName)
#endif
#ifndef CLASS_END
#define CLASS_END()
#endif

#include "cbase/reflect/CBaseEntity.h"
#include "cbase/reflect/CBaseDelay.h"
#include "cbase/reflect/CBaseAnimating.h"
#include "cbase/reflect/CBaseToggle.h"
#include "cbase/reflect/CBasePlayerItem.h"
#include "cbase/reflect/CBasePlayerWeapon.h"
#include "cbase/reflect/CBaseMonster.h"
#include "cbase/reflect/CBasePlayer.h"
#ifndef CLIENT_DLL
#include "cbase/reflect/CPointEntity.h"
#include "cbase/reflect/CArmoury.h"
#include "cbase/reflect/CGrenade.h"
#include "cbase/reflect/CBasePlayerAmmo.h"
#include "cbase/reflect/CWeaponBox.h"
#endif

#undef CLASS_BEGIN
#undef ADD_VIRTUAL_FUNCTION
#undef ADD_OVERRIDE_VIRTUAL_FUNCTION
#undef ADD_MEMBER_FUNCTION
#undef ADD_STATIC_MEMBER_FUNCTION
#undef ADD_MEMBER_DATA
#undef ADD_STATIC_MEMBER_DATA
#undef CLASS_END