#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

struct CBaseTemplateWeapon : CBasePlayerWeapon
{
	using Base = CBasePlayerWeapon;
	// ...
};

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
#include "RecoilKickBack.hpp"
#include "RecoilPunch.hpp"
#include "GetDamageDefault.hpp"

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