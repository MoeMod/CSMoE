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

#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#endif

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CM134HeroI : public LinkWeaponTemplate<CM134HeroI,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
	TPrecacheEvent,
	TDeployDefault,
	TReloadDefault,
	TSecondaryAttackZoom,
	TPrimaryAttackRifle,
	TFireRifle,
	TRecoilKickBack,
	TWeaponIdleDefault,
	TGetDamageDefault
>
{
public:
	static constexpr const auto &DefaultReloadTime = 4.8s;
	static constexpr int ZoomFOV = 55;
	static constexpr const char *V_Model = "models/z4b/v_m134heroi.mdl";
	static constexpr const char *P_Model = "models/z4b/p_m134heroi.mdl";
	static constexpr const char *W_Model = "models/z4b/w_m134heroi.mdl";
	static constexpr const char *EventFile = "events/m134heroi.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_m134heroi";
	static constexpr const char *AnimExtension = "m134";
	static constexpr int MaxClip = 200;

    enum
    {
        ANIM_IDLE1 = 0,
        ANIM_DRAW,
        ANIM_RELOAD,
        ANIM_FIRE_READY,
        ANIM_SHOOT,
        ANIM_FIRE_AFTER,
        ANIM_FIRE_CHANGE,
        ANIM_IDLE_CHANGE,
        ANIM_DRAW_OVERHEAT,
        ANIM_FIRE_AFTER_OVERHEAT,
        ANIM_IDLE_OVERHEAT,
        ANIM_OVERHEAT_END,
        ANIM_FIRE_READY_SHOOTB,
        ANIM_SHOOT2,
        ANIM_FIRE_AFTER_SHOOTB,
    };

	static constexpr const auto & SpreadCalcNotOnGround = 0.05 + 0.1 * A;
	static constexpr const auto & SpreadCalcWalking = 0.04 + 0.02 * A;
	static constexpr const auto & SpreadCalcDefault = 0.025 + 0.0125 * A;
	static constexpr const auto &CycleTime = 0.06s;
	static constexpr int DamageDefault = 33;
	static constexpr int DamageZB = 66;
	static constexpr int DamageZBS = 100;
	static constexpr const auto & AccuracyCalc = (N * N * N / 200.0) + 0.4;
	static constexpr float AccuracyDefault = 0.0;
	static constexpr float AccuracyMax = 2.5;
	static constexpr float RangeModifier = 0.95;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	KickBackData KickBackWalking = { 0.3f, 0.075f, 0.05f, 0.02f, 2.5f, 1.5f, 0 };
	KickBackData KickBackNotOnGround = { 1.0f, 0.4f, 0.2f, 0.015f, 3.0f, 2.0f, 0 };
	KickBackData KickBackDucking = { 0.175f ,0.04f , 0.03f, 0.01f, 1.5f, 1.0f, 0 };
	KickBackData KickBackDefault = { 0.2f, 0.05f, 0.035f, 0.015f, 2.0f, 1.25f, 0 };
	KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

public:
    static constexpr auto	EGON_PRIMARY_VOLUME	=	450;
    static constexpr const char *EGON_BEAM_SPRITE	=	"sprites/ef_gungnir_xbeam.spr";
    static constexpr const char *EGON_FLARE_SPRITE	=	"sprites/xspark1.spr";
    static constexpr const char *EGON_SOUND_OFF		=	"weapons/egon_off1.wav";
    static constexpr const char *EGON_SOUND_RUN		=	"weapons/egon_run3.wav";
    static constexpr const char *EGON_SOUND_STARTUP	=	"weapons/egon_windup2.wav";

public:
    enum {
        FIRE_OFF,
        FIRE_CHARGE
    } m_fireState;
    enum {
        FIRE_NARROW,
        FIRE_WIDE
    } m_fireMode;
    unsigned short m_usEgonFire;
    unsigned short m_usEgonStop;
    bool m_deployed;

#ifndef CLIENT_DLL
    CBeam *m_pBeam;
    CBeam *m_pNoise;
    CSprite *m_pSprite;
#endif
    time_point_t m_flAmmoUseTime;
    time_point_t m_shakeTime;

public:
    BOOL Deploy() override;
    void Holster( int skiplocal /* = 0 */ ) override { EndAttack(); }
    void Precache() override;
    void SecondaryAttack() override;
    void WeaponIdle( ) override;

    void SpecialAttack();
    void SpecialFire(const Vector &vecOrigSrc, const Vector &vecDir);
    void EndAttack();

    void CreateEffect();
    void DestroyEffect();
    void UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend, bool bHit=false );

    BOOL HasAmmo();
    void UseAmmo(int);

    duration_t GetPulseInterval(  )
    {
        return 0.1s;
    }

    duration_t GetDischargeInterval(  )
    {
        return 0.02s;
    }

    float GetSecondaryAttackDamage() const
    {
        float flDamage = 11;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 22;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 80;
#endif
        return flDamage;
    }
};

LINK_ENTITY_TO_CLASS(z4b_m134heroi, CM134HeroI)

void CM134HeroI::Precache()
{
    Base::Precache();
    m_usEgonFire = PRECACHE_EVENT( 1, "events/m134heroi_fire.sc" );
    m_usEgonStop = PRECACHE_EVENT( 1, "events/m134heroi_stop.sc" );
    PRECACHE_SOUND( EGON_SOUND_OFF );
    PRECACHE_SOUND( EGON_SOUND_RUN );
    PRECACHE_SOUND( EGON_SOUND_STARTUP );

    PRECACHE_MODEL( EGON_BEAM_SPRITE );
    PRECACHE_MODEL( EGON_FLARE_SPRITE );
}

BOOL CM134HeroI::Deploy()
{
    m_deployed = FALSE;
    m_fireState = FIRE_OFF;
    return Base::Deploy();
}

void CM134HeroI::WeaponIdle( void )
{
    ResetEmptySound();

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
        return;

    if( m_fireState != FIRE_OFF )
        EndAttack();

    int iAnim;

    iAnim = ANIM_IDLE1;
    SendWeaponAnim( ANIM_IDLE1 );
    m_deployed = TRUE;
}

void CM134HeroI::EndAttack(  )
{
    bool bMakeNoise = false;

    if( m_fireState != FIRE_OFF ) //Checking the button just in case!.
        bMakeNoise = true;

    PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, m_pPlayer->edict(), m_usEgonStop, 0, m_pPlayer->pev->origin, m_pPlayer->pev->angles, 0.0, 0.0, bMakeNoise, 0, 0, 0 );

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;

    m_fireState = FIRE_OFF;

    DestroyEffect();
}

void CM134HeroI::CreateEffect(  )
{
#ifndef CLIENT_DLL
    DestroyEffect();

    m_pBeam = CBeam::BeamCreate( EGON_BEAM_SPRITE, 40 );
    m_pBeam->PointEntInit( pev->origin, m_pPlayer->entindex() );
    m_pBeam->SetFlags( BEAM_FSINE );
    //m_pBeam->SetEndAttachment( 1 );
    m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
    //m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
    m_pBeam->pev->owner = m_pPlayer->edict();

    m_pNoise = CBeam::BeamCreate( EGON_BEAM_SPRITE, 55 );
    m_pNoise->PointEntInit( pev->origin, m_pPlayer->entindex() );
    m_pNoise->SetScrollRate( 25 );
    m_pNoise->SetBrightness( 100 );
    m_pNoise->SetEndAttachment( 1 );
    m_pNoise->pev->spawnflags |= SF_BEAM_TEMPORARY;
    m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
    m_pNoise->pev->owner = m_pPlayer->edict();

    m_pSprite = CSprite::SpriteCreate( EGON_FLARE_SPRITE, pev->origin, FALSE );
    m_pSprite->pev->scale = 1.0;
    m_pSprite->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
    m_pSprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;
    //m_pSprite->pev->flags |= FL_SKIPLOCALHOST;
    m_pSprite->pev->owner = m_pPlayer->edict();

    if( m_fireMode == FIRE_WIDE )
    {
        m_pBeam->SetScrollRate( 50 );
        m_pBeam->SetNoise( 20 );
        m_pNoise->SetColor( 50, 50, 255 );
        m_pNoise->SetNoise( 8 );
    }
    else
    {
        m_pBeam->SetScrollRate( 110 );
        m_pBeam->SetNoise( 5 );
        m_pNoise->SetColor( 80, 120, 255 );
        m_pNoise->SetNoise( 2 );
    }
#endif
}

void CM134HeroI::DestroyEffect(  )
{
#ifndef CLIENT_DLL
    if( m_pBeam )
    {
        UTIL_Remove( m_pBeam );
        m_pBeam = nullptr;
    }

    if( m_pNoise )
    {
        UTIL_Remove( m_pNoise );
        m_pNoise = nullptr;
    }
    if( m_pSprite )
    {
        if( m_fireMode == FIRE_WIDE )
            m_pSprite->Expand( 10, 500 );
        else
            UTIL_Remove( m_pSprite );
        m_pSprite = nullptr;
    }
#endif
}

void CM134HeroI::SpecialAttack(  )
{
    // don't fire underwater
    if( m_pPlayer->pev->waterlevel == 3 )
    {
#ifndef CLIENT_DLL
        if( m_fireState != FIRE_OFF || m_pBeam )
        {
            EndAttack();
        }
        else
#endif
        {
            PlayEmptySound();
        }
        return;
    }

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
    Vector vecAiming = gpGlobals->v_forward;
    Vector vecSrc = m_pPlayer->GetGunPosition();

    int flags;
//#if defined( CLIENT_WEAPONS )
//    flags = FEV_NOTHOST;
//#else
    flags = 0;
//#endif

    switch( m_fireState )
    {
        case FIRE_OFF:
        {
            if( !HasAmmo() )
            {
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25s;
                PlayEmptySound( );
                return;
            }

            m_flAmmoUseTime = gpGlobals->time;// start using ammo ASAP.

            PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fireMode, 1, 0 );

            m_shakeTime = invalid_time_point;

            m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
            pev->fuser1 = (UTIL_WeaponTimeBase() + 2s) / 1s;

            pev->dmgtime = gpGlobals->time + GetPulseInterval();
            m_fireState = FIRE_CHARGE;
            break;
        }
        case FIRE_CHARGE:
        {
            SpecialFire( vecSrc, vecAiming );
            m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;

            if( pev->fuser1 <= UTIL_WeaponTimeBase() / 1s )
            {
                PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fireMode, 0, 0 );
                pev->fuser1 = 1000;
            }
            else
            {
                PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEgonFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fireState, m_fireMode, 0, 1 );
            }

            if( !HasAmmo() )
            {
                EndAttack();
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
            }
            break;
        }
    }
}

void CM134HeroI::SecondaryAttack(  )
{
    m_fireMode = FIRE_WIDE;
    SpecialAttack();
}

void CM134HeroI::SpecialFire( const Vector &vecOrigSrc, const Vector &vecDir )
{
    Vector vecDest = vecOrigSrc + vecDir * 2048;
    edict_t *pentIgnore;
    TraceResult tr;

    pentIgnore = m_pPlayer->edict();
    Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

    // ALERT( at_console, "." );

    UTIL_TraceLine( vecOrigSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );

    if( tr.fAllSolid )
        return;

#ifndef CLIENT_DLL
    CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

    if( pEntity == nullptr)
        return;

    if(!pEntity->pev->takedamage)
    {
        UTIL_TraceHull( vecOrigSrc, vecDest, dont_ignore_monsters, head_hull, pentIgnore, &tr );
        pEntity = CBaseEntity::Instance( tr.pHit );
        if(!pEntity->pev->takedamage)
        {
            UTIL_TraceHull( vecOrigSrc, vecDest, dont_ignore_monsters, human_hull, pentIgnore, &tr );
            pEntity = CBaseEntity::Instance( tr.pHit );
        }
    }

    if(g_pGameRules->IsMultiplayer() )
    {
        if( m_pSprite && pEntity->pev->takedamage )
        {
            m_pSprite->pev->effects &= ~EF_NODRAW;
        }
        else if( m_pSprite )
        {
            m_pSprite->pev->effects |= EF_NODRAW;
        }
    }
#endif
    float timedist = 0.0f;

    switch( m_fireMode )
    {
        case FIRE_NARROW:
#ifndef CLIENT_DLL
            if( pev->dmgtime < gpGlobals->time )
            {
                // Narrow mode only does damage to the entity it hits
                ClearMultiDamage();
                if( pEntity->pev->takedamage )
                {
                    pEntity->TraceAttack( m_pPlayer->pev, GetSecondaryAttackDamage(), vecDir, &tr, DMG_ENERGYBEAM );
                }
                ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

                if( g_pGameRules->IsMultiplayer() )
                {
                    // multiplayer uses 1 ammo every 1/10th second
                    if( gpGlobals->time >= m_flAmmoUseTime )
                    {
                        UseAmmo( 1 );
                        m_flAmmoUseTime = gpGlobals->time + 0.1s;
                    }
                }
                else
                {
                    // single player, use 3 ammo/second
                    if( gpGlobals->time >= m_flAmmoUseTime )
                    {
                        UseAmmo( 1 );
                        m_flAmmoUseTime = gpGlobals->time + 0.166s;
                    }
                }

                pev->dmgtime = gpGlobals->time + GetPulseInterval();
            }
#endif
            timedist = ( pev->dmgtime - gpGlobals->time ) / GetPulseInterval();
            break;
        case FIRE_WIDE:
#ifndef CLIENT_DLL
            if( pev->dmgtime < gpGlobals->time )
            {
                // wide mode does damage to the ent, and radius damage
                ClearMultiDamage();
                if( pEntity->pev->takedamage )
                {
                    pEntity->TraceAttack( m_pPlayer->pev, GetSecondaryAttackDamage(), vecDir, &tr, DMG_BULLET | DMG_ENERGYBEAM );
                }
                ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

                if( g_pGameRules->IsMultiplayer() )
                {
                    // radius damage a little more potent in multiplayer.
                    sv::RadiusDamage( tr.vecEndPos, pev, m_pPlayer->pev, GetSecondaryAttackDamage()/4, 128, CLASS_NONE, DMG_BULLET | DMG_ENERGYBEAM );
                }

                if( !m_pPlayer->IsAlive() )
                    return;

                if( g_pGameRules->IsMultiplayer() )
                {
                    //multiplayer uses 5 ammo/second
                    if( gpGlobals->time >= m_flAmmoUseTime )
                    {
                        UseAmmo( 1 );
                        m_flAmmoUseTime = gpGlobals->time + 0.2s;
                    }
                }
                else
                {
                    // Wide mode uses 10 charges per second in single player
                    if( gpGlobals->time >= m_flAmmoUseTime )
                    {
                        UseAmmo( 1 );
                        m_flAmmoUseTime = gpGlobals->time + 0.1s;
                    }
                }

                pev->dmgtime = gpGlobals->time + GetDischargeInterval();
                if( m_shakeTime < gpGlobals->time )
                {
                    UTIL_ScreenShake( tr.vecEndPos, 5.0, 150.0, 0.75, 250.0 );
                    m_shakeTime = gpGlobals->time + 0.25s;
                }
            }
#endif
            timedist = ( pev->dmgtime - gpGlobals->time ) / GetDischargeInterval();
            break;
    }

    if( timedist < 0 )
        timedist = 0;
    else if( timedist > 1 )
        timedist = 1;
    timedist = 1 - timedist;

#ifndef CLIENT_DLL
    if(pEntity && pEntity->pev->takedamage)
        UpdateEffect( tmpSrc, tr.vecEndPos, timedist, true );
    else
#endif
        UpdateEffect( tmpSrc, tr.vecEndPos, timedist );
}

BOOL CM134HeroI::HasAmmo()
{
    return m_iClip > 0;
}

void CM134HeroI::UseAmmo( int count )
{
    if( m_iClip >= count )
        m_iClip -= count;
    else
        m_iClip = 0;
}

void CM134HeroI::UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend, bool bHit )
{
#ifndef CLIENT_DLL
    if( !m_pBeam )
    {
        CreateEffect();
    }

    m_pBeam->SetStartPos( endPoint );
    m_pBeam->SetBrightness( (int)( 255 - ( timeBlend * 180 )) );
    m_pBeam->SetWidth( (int)( 40 - ( timeBlend * 20 ) ) );

    if(bHit)
    {
        if( m_fireMode == FIRE_WIDE )
            m_pBeam->SetColor( (int)( 64 + ( 25 * timeBlend ) ), (int)( 72 + ( 30 * timeBlend ) ), (int)( 30 + 25 * fabs( sin( gpGlobals->time.time_since_epoch().count() * 10 ) ) ) );
        else
            m_pBeam->SetColor( (int)( 128 + ( 25 * timeBlend ) ), (int)( 144 + ( 30 * timeBlend ) ), (int)( 30 + 25 * fabs( sin( gpGlobals->time.time_since_epoch().count() * 10 ) ) ) );
    }
    else
    {
        if( m_fireMode == FIRE_WIDE )
            m_pBeam->SetColor( (int)( 30 + ( 25 * timeBlend ) ), (int)( 30 + ( 30 * timeBlend ) ), (int)( 64 + 80 * fabs( sin( gpGlobals->time.time_since_epoch().count() * 10 ) ) ) );
        else
            m_pBeam->SetColor( (int)( 60 + ( 25 * timeBlend ) ), (int)( 120 + ( 30 * timeBlend ) ), (int)( 64 + 80 * fabs( sin( gpGlobals->time.time_since_epoch().count() * 10 ) ) ) );
    }

    UTIL_SetOrigin( m_pSprite->pev, endPoint );
    m_pSprite->pev->frame += 8 * gpGlobals->frametime / 1s;
    if( m_pSprite->pev->frame > m_pSprite->Frames() )
        m_pSprite->pev->frame = 0;

    m_pNoise->SetStartPos( endPoint );
#endif
}

}
