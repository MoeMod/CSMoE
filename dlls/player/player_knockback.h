#ifndef PLAYER_KNOCKBACK_H
#define PLAYER_KNOCKBACK_H
#ifdef _WIN32
#pragma once
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
class Vector; // vector.h
void ApplyKnockbackData(CBasePlayer *player, const Vector &dir, const KnockbackData &data);

#endif