#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "player_knockback.h"

namespace sv {

// copied from z4e_knockback.sma - KnockBack_Set
void ApplyKnockbackData(CBasePlayer *player, const Vector &dir, const KnockbackData &data)
{
	float flKnockbackRatio = 0.0f;
	float flVelocityModifier = data.flVelocityModifier;

	Vector2D dir2 = dir.Make2D().Normalize();
	Vector vecVelocityDir = player->pev->velocity.Normalize();

	if (player->pev->flags & FL_ONGROUND)
	{
		if (player->pev->flags & FL_DUCKING || player->m_LastHitGroup == HITGROUP_LEFTLEG || player->m_LastHitGroup == HITGROUP_RIGHTLEG)
			flKnockbackRatio = data.flDucking;
		else
			flKnockbackRatio = data.flOnGround;
	}
	else
	{
		if (player->pev->velocity.Length2D() > 0.0f)
			flKnockbackRatio = data.flFlying * flVelocityModifier;
		else
			flKnockbackRatio = data.flNotOnGround * flVelocityModifier;
	}

	if (DotProduct(dir.Normalize(), vecVelocityDir) > 0)
		flKnockbackRatio = -1.0;
	// TBD : zombie class konckback ratio 
	//flKnockbackRatio *= 0.5f;

	if (flVelocityModifier > 0.0)
	{
		player->m_flVelocityModifier = flVelocityModifier;
	}

	if (flKnockbackRatio > -1.0f)
	{
		//player->pev->velocity = fma(dir2, flKnockbackRatio, player->pev->velocity);
		player->pev->velocity.x = flKnockbackRatio * dir2.x;
		player->pev->velocity.y = flKnockbackRatio * dir2.y;
	}
}

}
