/*
WeaponTemplate.hpp - part of CSMoE template weapon framework,
                    include all headers,
                    provide with basic meta-functions
					based on template base class, CRTP and SFINAE
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

struct CBaseTemplateWeapon : CBasePlayerWeapon
{
	using Base = CBasePlayerWeapon;
	// ...
};

#include "WeaponTemplateDetails.hpp"
#include "WeaponTemplateDataFields.hpp"
#include "WeaponDataVaribles.hpp"
#include "CheckAccuracyBoundary.hpp"
#include "GeneralData.hpp"
#include "PrecacheEvent.hpp"
#include "ItemInfo.hpp"
#include "DeployDefault.hpp"
#include "ReloadDefault.hpp"
#include "PrimaryAttackRifle.hpp"
#include "SecondaryAttackZoom.hpp"
#include "SecondaryAttackSniperZoom1.hpp"
#include "SecondaryAttackSniperZoom2.hpp"
#include "WeaponIdleDefault.hpp"
#include "FireRifle.hpp"
#include "FirePistol.hpp"
#include "RecoilKickBack.hpp"
#include "RecoilPunch.hpp"
#include "GetDamageDefault.hpp"
#include "DoubleModeType.hpp"

/*
 * template<class CFinal, class CBase>
 * concept TWeaponNode;
 */

template<class CFinal, template<class, class> class First, template<class, class> class...Args>
struct LinkWeaponTemplate : First<CFinal, LinkWeaponTemplate<CFinal, Args...>>
{
	using Base = LinkWeaponTemplate;
	using Final = CFinal;
	// ...
};

template<class CFinal, template<class, class> class First>
struct LinkWeaponTemplate<CFinal, First> : First<CFinal, CBaseTemplateWeapon>
{
	using Base = LinkWeaponTemplate;
	using Final = CFinal;
	// ...
};

// CFinal--A--B--C--D--E--...
// typename FindDownSideDerivedClass<typename B::Final, B>::type => A
// typename B::Base => C
template<class CFinal, class CCurrent, class Last=void>
struct FindDownSideDerivedClass
{
	using type = typename FindDownSideDerivedClass<typename CFinal::Base, CCurrent, CFinal>::type;
};
template<class CCurrent, class CLast>
struct FindDownSideDerivedClass<CCurrent, CCurrent, CLast>
{
	using type = CLast;
};

}