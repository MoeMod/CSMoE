#ifndef PLAYER_KNOCKBACK_H
#define PLAYER_KNOCKBACK_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

struct KnockbackData
{
	float flOnGround;
	float flNotOnGround;
	float flFlying;
	float flDucking;
	float flVelocityModifier;
};

class CBasePlayer; // player.h

void ApplyKnockbackData(CBasePlayer *player, const Vector &dir, const KnockbackData &data);

}

#endif