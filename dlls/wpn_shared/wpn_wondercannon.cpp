/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define WPNSTATE_WINGGUN_WINGON	(1<<1)

    class CWingGun : public LinkWeaponTemplate< CWingGun,
        TGeneralData,
        BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
        TPrecacheEvent,
        TDeployDefault
    >
    {
    public:
        static constexpr const char* V_Model = "models/v_winggun.mdl";
        static constexpr const char* P_Model = "models/p_winggun.mdl";
        static constexpr const char* W_Model = "models/w_winggun.mdl";
        static constexpr const char* EventFile = "events/winggun.sc";
        static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
        static constexpr const char* ClassName = "weapon_winggun";
        static constexpr const char* AnimExtension = "m249";
        static constexpr int MaxClip = 50;
        enum
        {
            ANIM_IDLE,
            ANIM_SHOOT1,
            ANIM_SHOOT2,
            ANIM_RELOAD,
            ANIM_DRAW,
            ANIM_WINGSTART,
            ANIM_WINGLOOP,
            ANIM_WINGEND,
            ANIM_WINGSHOOT1,
            ANIM_WINGSHOOT2,
            ANIM_WINGRELOAD,//USELESS
            ANIM_WINGSPECIALSHOT,
            ANIM_WINGRELOAD2

        };
        static constexpr float PlayerMaxSpeed = 205;
        static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
        static constexpr int Penetration = 1;
        KnockbackData KnockBack = { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };