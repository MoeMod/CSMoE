#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "player_knockback.h"

// copied from z4e_knockback.sma - KnockBack_Set
void ApplyKnockbackData(CBasePlayer *player, const Vector &dir, const KnockbackData &data)
{
	float flKnockbackRatio = 0.0f;
	float flVelocityModifier = data.flVelocityModifier;

	if (player->pev->flags & FL_ONGROUND)
	{
		if (player->pev->flags & FL_DUCKING)
			flKnockbackRatio = data.flDucking;
		else
			flKnockbackRatio = data.flOnGround;
	}
	else
	{
		if (player->pev->velocity.Length2D() > 140.f)
			flKnockbackRatio = data.flFlying;
		else
			flKnockbackRatio = data.flNotOnGround;
	}
	// TBD : zombie class konckback ratio 
	//flKnockbackRatio *= 0.5f;

	if (flKnockbackRatio > 0.0f)
	{
		Vector dir2 = dir.Normalize();
		dir2.z = 0.0f;
		player->pev->velocity = player->pev->velocity + dir2 * flKnockbackRatio;
	}

	if (flVelocityModifier > 0.0)
	{
		player->m_flVelocityModifier = flVelocityModifier;
	}
}