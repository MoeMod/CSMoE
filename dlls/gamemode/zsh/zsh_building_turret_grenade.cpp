#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "schedule.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"

#include "gamemode/mods.h"

#include "zsh_building_turret_grenade.h"

namespace sv
{

class CZSHTurretGrenade : public CBaseEntity
{
public:
    void Precache() override
    {
        PRECACHE_MODEL("models/grenade.mdl");
        m_ExpIndex = PRECACHE_MODEL("sprites/zerogxplode.spr");
    }
    void Spawn() override
    {
        Precache();

        SetThink(&CZSHTurretGrenade::FlyThink);
        SetTouch(&CZSHTurretGrenade::OnTouch);
        SET_MODEL(this->edict(), "models/grenade.mdl");

        pev->rendermode = kRenderNormal;
        pev->renderfx = kRenderFxNone;
        pev->renderamt = 255;
        pev->framerate = 1.0;
        pev->solid = SOLID_BBOX;
        pev->movetype = MOVETYPE_FLY;

        UTIL_SetSize(pev, g_vecZero, g_vecZero);

        pev->classname = MAKE_STRING("zsh_turret_grenade");
        pev->nextthink = gpGlobals->time + 0.01s;
    }

    void EXPORT OnTouch(CBaseEntity* pOther)
    {
        if(m_bToSky && UTIL_PointContents(pev->origin) == CONTENTS_SKY)
        {
            TraceResult result;
            Vector ceiling = m_vecTarget;
            ceiling.z += 8192.0;
            UTIL_TraceLine(m_vecTarget, ceiling, ignore_monsters, dont_ignore_glass, ENT(pev), &result);
            CZSHTurretGrenade::Create(m_hPlayer, pev->owner, result.vecEndPos, m_vecTarget, false);
            Remove();
            return;
        }

        RadiusDamage();
        Remove();
    }

    void Remove()
    {
        SetThink(nullptr);
        SetTouch(nullptr);
        pev->effects |= EF_NODRAW;
        UTIL_Remove(this);
    }

    void RadiusDamage()
    {
        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_EXPLOSION);
        WRITE_COORD(pev->origin.x);
        WRITE_COORD(pev->origin.y);
        WRITE_COORD(pev->origin.z + 30.0f);
        WRITE_SHORT(m_ExpIndex);
        WRITE_BYTE(RANDOM_LONG(15, 35));
        WRITE_BYTE(20);
        WRITE_BYTE(TE_EXPLFLAG_NONE);
        MESSAGE_END();

        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 300.0f)) != NULL)
        {
            if (!pEntity->IsAlive()) continue;
            if (pEntity->pev->takedamage == DAMAGE_NO) continue;
            if (pEntity->IsPlayer()) continue; 
            if (!FClassnameIs(pEntity->pev, "monster_entity")) continue;
            
            float flCurrentDamage = 150.0f - 150.0f / 300.0f * (pev->origin - pEntity->pev->origin).Length();
            pEntity->TakeDamage(pev, m_hPlayer ? m_hPlayer->pev : nullptr, flCurrentDamage, DMG_EXPLOSION);
        }
    }

    void EXPORT FlyThink()
    {
        pev->nextthink = gpGlobals->time + 0.5s;
    }

    static void Create(CBasePlayer *pPlayer, edict_t* owner, Vector origin, Vector target, bool bToSky)
    {
        CZSHTurretGrenade* pEntity = CreateClassPtr<CZSHTurretGrenade>();
        pEntity->Spawn();
        UTIL_SetOrigin(pEntity->pev, origin);
        pEntity->m_hPlayer = pPlayer;
        pEntity->pev->owner = owner;
        pEntity->m_bToSky = bToSky;
        pEntity->m_vecTarget = target;

        Vector vecVelocity = Vector(0, 0, bToSky ? 2000 : -2000);
        Vector vecAngles = UTIL_VecToAngles(vecVelocity);
        pEntity->pev->angles = vecAngles;
        pEntity->pev->velocity = vecVelocity;
    }

    Vector m_vecTarget;
    int m_ExpIndex;
    bool m_bToSky;
    EntityHandle<CBasePlayer> m_hPlayer;
};
LINK_ENTITY_TO_CLASS(zsh_turret_grenade, CZSHTurretGrenade)

void CBuilding_TurretGrenade::OnCreate()
{
    PRECACHE_MODEL("models/zsh_building/zsh_turret.mdl");
    SET_MODEL(m_pBuilding->edict(), "models/zsh_building/zsh_turret.mdl");
    m_vecMins = Vector(-40.0, -40.0, 0.0);
    m_vecMaxs = Vector(40.0, 40.0, 65.0);
	m_pBuilding->m_flFieldOfView = VIEW_FIELD_ULTRA_NARROW;
    m_flTurnRate = 15.0f;
    m_flVerticalTurnRate = 3.0f;
    m_iRange = 2000;
    m_iFireTimes = 0;
    m_flFireRate = invalid_time_point;
    m_iFireTimes = 0;
}

void CBuilding_TurretGrenade::OnCompleted()
{
    m_vecCurAngles = m_vecGoalAngles = m_pBuilding->pev->angles;
}

void CBuilding_TurretGrenade::OnThink()
{
    if (!m_pBuilding->isCompleted()) return;

    m_pBuilding->pev->nextthink = gpGlobals->time + 0.1s;

    if (m_iFireTimes > 0) {
        if (m_flFireRate > gpGlobals->time)
            return;

        m_iFireTimes--;
        m_flFireRate = gpGlobals->time + 0.7s;
        Shoot();
        return;
    }

    Search();

    if (m_pBuilding->m_hEnemy)
    {
        Active();
    }
    else
    {
        Retire();
    }
}

void CBuilding_TurretGrenade::Search()
{
    // If we have a target and we're still healthy
    if (m_pBuilding->m_hEnemy)
    {
        if (!m_pBuilding->m_hEnemy->IsAlive())
            m_pBuilding->m_hEnemy = NULL;// Dead enemy forces a search for new one
    }

    if (m_pBuilding->m_hEnemy) return;

    // Acquire Target
    m_pBuilding->m_hEnemy = BestVisibleEnemy();
}

void CBuilding_TurretGrenade::Active()
{
    auto pev = m_pBuilding->pev;
    Vector vecMid = pev->origin + pev->view_ofs;
    Vector vecMidEnemy = m_pBuilding->m_hEnemy->BodyTarget(vecMid);
    Vector vecDirToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
    float flDistToEnemy = vecDirToEnemy.LengthSquared();

    // Current enmey is not visible.

    if ((flDistToEnemy > m_iRange * m_iRange))
    {
        m_pBuilding->m_hEnemy = NULL;
        return;
    }

    Vector vec = UTIL_VecToAngles(vecDirToEnemy);
    vec.x = 90;

    // [1...360]
    if (vec.y > 360)
        vec.y -= 360;
    if (vec.y < 0)
        vec.y += 360;

    m_vecGoalAngles.y = vec.y;
    m_vecGoalAngles.x = vec.x;
    MoveTurret();

    if (m_vecCurAngles.x < 90)
        return;

    vec = m_vecCurAngles;
    vec.x = 0.0f;
    UTIL_MakeVectors(vec);
    if (DotProduct(vecDirToEnemy.Make2D().Normalize(), gpGlobals->v_forward.Make2D()) <= m_pBuilding->m_flFieldOfView)
        return;

    m_vecTarget = vecMidEnemy;
    m_iFireTimes = 4;
}

void CBuilding_TurretGrenade::Shoot()
{
    auto ent = ENT(m_pBuilding->pev);
    Vector vecSrc, vecAngles;
    m_pBuilding->GetAttachment(0, vecSrc, vecAngles);
    CZSHTurretGrenade::Create(m_pBuilding->m_hBuilder, ent, vecSrc, m_vecTarget, true);
    EMIT_SOUND(ent, CHAN_WEAPON, "weapons/petrolboomer-1.wav", 1, ATTN_NORM);
}

void CBuilding_TurretGrenade::Retire()
{
    m_vecGoalAngles.x = 0;
    MoveTurret();
}

CBaseEntity* CBuilding_TurretGrenade::BestVisibleEnemy()
{
    CBaseEntity* pReturn;
    int iNearest;
    int iDist;

    iNearest = 67108864; // 8192*8192
    pReturn = NULL;

    CBaseEntity* pEntity = nullptr;
    while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pBuilding->pev->origin, m_iRange)) != nullptr) {
        if (!pEntity->IsAlive()) continue;
        if (pEntity->IsPlayer()) continue;
        if (!FClassnameIs(pEntity->pev, "monster_entity")) continue;

        iDist = (pEntity->pev->origin - m_pBuilding->pev->origin).LengthSquared();
        if (iDist <= iNearest)
        {
            iNearest = iDist;
            pReturn = pEntity;
        }
    }

    return pReturn;
}

bool CBuilding_TurretGrenade::MoveTurret()
{
    bool state = false;
    if (m_vecCurAngles.x != m_vecGoalAngles.x)
    {
        float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1;

        m_vecCurAngles.x += m_flVerticalTurnRate * flDir;

        // if we started below the goal, and now we're past, peg to goal
        if (flDir == 1)
        {
            if (m_vecCurAngles.x > m_vecGoalAngles.x)
                m_vecCurAngles.x = m_vecGoalAngles.x;
        }
        else
        {
            if (m_vecCurAngles.x < m_vecGoalAngles.x)
                m_vecCurAngles.x = m_vecGoalAngles.x;
        }

        m_pBuilding->SetBoneController(1, m_vecCurAngles.x);
        state = true;
    }

    if (m_vecCurAngles.y != m_vecGoalAngles.y)
    {
        float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1;
        float flDist = fabs(m_vecGoalAngles.y - m_vecCurAngles.y);

        // 反方向更近
        if (flDist > 180)
        {
            flDist = 360 - flDist;
            flDir = -flDir;
        }

        m_vecCurAngles.y += m_flTurnRate * flDir;

        if (m_vecCurAngles.y < 0)
            m_vecCurAngles.y += 360;
        else if (m_vecCurAngles.y >= 360)
            m_vecCurAngles.y -= 360;

        if (flDist < m_flTurnRate)
            m_vecCurAngles.y = m_vecGoalAngles.y;

        m_pBuilding->SetBoneController(0, m_vecCurAngles.y - m_pBuilding->pev->angles.y);
        state = true;
    }

	return state;
}

void CBuilding_TurretGrenade::SetAnim(int iAnim)
{
	if(m_pBuilding->pev->sequence == iAnim) return;

	m_pBuilding->pev->frame = 0;
	m_pBuilding->pev->sequence = iAnim;
	m_pBuilding->ResetSequenceInfo();
}

}
