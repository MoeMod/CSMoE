#pragma once

// for client weapons prediction only.
// NO NEED to include me in server dll

#include "wpn_shared/wpn_usp.h"
#include "wpn_shared/wpn_mp5navy.h"
#include "wpn_shared/wpn_sg552.h"
#include "wpn_shared/wpn_ak47.h"
#include "wpn_shared/wpn_aug.h"
#include "wpn_shared/wpn_awp.h"
#include "wpn_shared/wpn_c4.h"
#include "wpn_shared/wpn_deagle.h"
#include "wpn_shared/wpn_flashbang.h"
#include "wpn_shared/wpn_g3sg1.h"
#include "wpn_shared/wpn_glock18.h"
#include "wpn_shared/wpn_hegrenade.h"
#include "wpn_shared/wpn_knife.h"
#include "wpn_shared/wpn_m249.h"
#include "wpn_shared/wpn_m3.h"
#include "wpn_shared/wpn_m4a1.h"
#include "wpn_shared/wpn_mac10.h"
#include "wpn_shared/wpn_p228.h"
#include "wpn_shared/wpn_p90.h"
#include "wpn_shared/wpn_scout.h"
#include "wpn_shared/wpn_smokegrenade.h"
#include "wpn_shared/wpn_tmp.h"
#include "wpn_shared/wpn_xm1014.h"
#include "wpn_shared/wpn_elite.h"
#include "wpn_shared/wpn_fiveseven.h"
#include "wpn_shared/wpn_ump45.h"
#include "wpn_shared/wpn_sg550.h"
#include "wpn_shared/wpn_galil.h"
#include "wpn_shared/wpn_famas.h"

// mapping CSW_XXX with CXXX using templates
template<WeaponIdType CSW, class WeaponClass> struct WeaponMapNode {};
template<class...> struct WeaponMap_Template {};
using WeaponMap = WeaponMap_Template<
	WeaponMapNode<WEAPON_P228, CP228>,
	WeaponMapNode<WEAPON_SCOUT, CSCOUT>,
	WeaponMapNode<WEAPON_HEGRENADE, CHEGrenade>,
	WeaponMapNode<WEAPON_XM1014, CXM1014>,
	WeaponMapNode<WEAPON_C4, CC4>,
	WeaponMapNode<WEAPON_MAC10, CMAC10>,
	WeaponMapNode<WEAPON_AUG, CAUG>,
	WeaponMapNode<WEAPON_SMOKEGRENADE, CSmokeGrenade>,
	WeaponMapNode<WEAPON_ELITE, CELITE>,
	WeaponMapNode<WEAPON_FIVESEVEN, CFiveSeven>,
	WeaponMapNode<WEAPON_UMP45, CUMP45>,
	WeaponMapNode<WEAPON_SG550, CSG550>,
	WeaponMapNode<WEAPON_GALIL, CGalil>,
	WeaponMapNode<WEAPON_FAMAS, CFamas>,
	WeaponMapNode<WEAPON_USP, CUSP>,
	WeaponMapNode<WEAPON_GLOCK18, CGLOCK18>,
	WeaponMapNode<WEAPON_AWP, CAWP>,
	WeaponMapNode<WEAPON_MP5N, CMP5N>,
	WeaponMapNode<WEAPON_M249, CM249>,
	WeaponMapNode<WEAPON_M3, CM3>,
	WeaponMapNode<WEAPON_M4A1, CM4A1>,
	WeaponMapNode<WEAPON_TMP, CTMP>,
	WeaponMapNode<WEAPON_G3SG1, CG3SG1>,
	WeaponMapNode<WEAPON_FLASHBANG, CFlashbang>,
	WeaponMapNode<WEAPON_DEAGLE, CDEAGLE>,
	WeaponMapNode<WEAPON_SG552, CSG552>,
	WeaponMapNode<WEAPON_AK47, CAK47>,
	WeaponMapNode<WEAPON_KNIFE, CKnife>,
	WeaponMapNode<WEAPON_P90, CP90>
>;

template<WeaponIdType CSW_to_find, class Map = WeaponMap>
struct WeaponIdToClass_MetaFunc;
template<WeaponIdType CSW_to_find, template<class...> class map_template, class Node1, class...Nodes>
struct WeaponIdToClass_MetaFunc<CSW_to_find, map_template<Node1, Nodes...>>
{
	using type = typename WeaponIdToClass_MetaFunc<CSW_to_find, map_template<Nodes...>>::type; //  not matched, continue...
};
template<WeaponIdType CSW_to_find, template<class...> class map_template, class T, class...Nodes>
struct WeaponIdToClass_MetaFunc<CSW_to_find, map_template<WeaponMapNode<CSW_to_find, T>, Nodes...>>
{
	using type = T; //  found it !!!
};
template<WeaponIdType CSW_to_find>
using WeaponIdToClass = typename WeaponIdToClass_MetaFunc<CSW_to_find>::type;
