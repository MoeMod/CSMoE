#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "DeployDefault.hpp"

#include "ReloadDefault.hpp"

#include "SecondaryAttackZoom.hpp"
#include "SecondaryAttackSniperZoom1.hpp"
#include "SecondaryAttackSniperZoom2.hpp"

#include "WeaponIdleDefault.hpp"

struct CBaseTemplateWeapon : CBasePlayerWeapon
{
	// ...
};

/*
 * template<class CFinal, class CBase>
 * concept TWeaponNode;
 */

template<class CFinal, template<class, class> class First, template<class, class> class...Args>
struct LinkWeaponTemplate : First<CFinal, LinkWeaponTemplate<CFinal, Args...>>
{
	// ...
};

template<class CFinal, template<class, class> class First>
struct LinkWeaponTemplate<CFinal, First> : First<CFinal, CBaseTemplateWeapon>
{
	// ...
};