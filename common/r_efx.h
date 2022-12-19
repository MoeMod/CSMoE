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
#if !defined ( R_EFXH )
#define R_EFXH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

// particle_t
#if !defined( PARTICLEDEF_H )  
#include "particledef.h"
#endif

// BEAM
#if !defined( BEAMDEF_H )
#include "beamdef.h"
#endif

// dlight_t
#if !defined ( DLIGHT_H )
#include "dlight.h"
#endif

// cl_entity_t
#if !defined( CL_ENTITY_H )
#include "cl_entity.h"
#endif

/*
// FOR REFERENCE, These are the built-in tracer colors.  Note, color 4 is the one
//  that uses the tracerred/tracergreen/tracerblue and traceralpha cvar settings
color24 gTracerColors[] =
{
	{ 255, 255, 255 },		// White
	{ 255, 0, 0 },			// Red
	{ 0, 255, 0 },			// Green
	{ 0, 0, 255 },			// Blue
	{ 0, 0, 0 },			// Tracer default, filled in from cvars, etc.
	{ 255, 167, 17 },		// Yellow-orange sparks
	{ 255, 130, 90 },		// Yellowish streaks (garg)
	{ 55, 60, 144 },		// Blue egon streak
	{ 255, 130, 90 },		// More Yellowish streaks (garg)
	{ 255, 140, 90 },		// More Yellowish streaks (garg)
	{ 200, 130, 90 },		// More red streaks (garg)
	{ 255, 120, 70 },		// Darker red streaks (garg)
};
*/

// Temporary entity array
#define TENTPRIORITY_LOW	0
#define TENTPRIORITY_HIGH	1

// TEMPENTITY flags
#define	FTENT_NONE				0x00000000
#define	FTENT_SINEWAVE			0x00000001
#define	FTENT_GRAVITY			0x00000002
#define FTENT_ROTATE			0x00000004
#define	FTENT_SLOWGRAVITY		0x00000008
#define FTENT_SMOKETRAIL		0x00000010
#define FTENT_COLLIDEWORLD		0x00000020
#define FTENT_FLICKER			0x00000040
#define FTENT_FADEOUT			0x00000080
#define FTENT_SPRANIMATE		0x00000100
#define FTENT_HITSOUND			0x00000200
#define FTENT_SPIRAL			0x00000400
#define FTENT_SPRCYCLE			0x00000800
#define FTENT_COLLIDEALL		0x00001000 // will collide with world and slideboxes
#define FTENT_PERSIST			0x00002000 // tent is not removed when unable to draw 
#define FTENT_COLLIDEKILL		0x00004000 // tent is removed upon collision with anything
#define FTENT_PLYRATTACHMENT	0x00008000 // tent is attached to a player (owner)
#define FTENT_SPRANIMATELOOP	0x00010000 // animating sprite doesn't die when last frame is displayed
#define FTENT_SPARKSHOWER		0x00020000
#define FTENT_NOMODEL			0x00040000 // Doesn't have a model, never try to draw ( it just triggers other things )
#define FTENT_CLIENTCUSTOM		0x00080000 // Must specify callback.  Callback function is responsible for killing tempent and updating fields ( unless other flags specify how to do things )
#define FTENT_IGNOREGRAVITY		0x00200000 // CS1.6
#define FTENT_FADEIN			0x10000000 //CSAE
#define FTENT_VELOCITYANGLES	0x08000000 // calc angles by velocity when updating

typedef struct tempent_s	TEMPENTITY;
typedef struct tempent_s
{
	int			flags;
	float		die;
	float		frameMax;
	float		x;
	float		y;
	float		z;
	float		fadeSpeed;
	float       livetime;
	float		bounceFactor;
	int			hitSound;
	void		( *hitcallback )	( struct tempent_s *ent, struct pmtrace_s *ptr );
	void		( *callback )		( struct tempent_s *ent, float frametime, float currenttime );
	TEMPENTITY	*next;
	int			priority;
	short		clientIndex;	// if attached, this is the index of the client to stick to
								// if COLLIDEALL, this is the index of the client to ignore
								// TENTS with FTENT_PLYRATTACHMENT MUST set the clientindex! 

	vec3_t		tentOffset;		// if attached, client origin + tentOffset = tent origin.
	cl_entity_t	entity;

	// baseline.origin		- velocity
	// baseline.renderamt	- starting fadeout intensity
	// baseline.angles		- angle velocity
} TEMPENTITY;

typedef struct efx_api_s efx_api_t;

struct efx_api_s
{
	particle_t* (*R_AllocParticle)			(void (*callback) (struct particle_s* particle, float frametime));
	void		(*R_BlobExplosion)			(const vec3_t org);
	void		(*R_Blood)					(const vec3_t org, const vec3_t dir, int pcolor, int speed);
	void		(*R_BloodSprite)			(const vec3_t org, int colorindex, int modelIndex, int modelIndex2, float size);
	void		(*R_BloodStream)			(const vec3_t org, const vec3_t dir, int pcolor, int speed);
	void		(*R_BreakModel)				(const vec3_t pos, const vec3_t size, const vec3_t dir, float random, float life, int count, int modelIndex, char flags);
	void		(*R_Bubbles)				(const vec3_t mins, const vec3_t maxs, float height, int modelIndex, int count, float speed);
	void		(*R_BubbleTrail)			(const vec3_t start, const vec3_t end, float height, int modelIndex, int count, float speed);
	void		(*R_BulletImpactParticles)	(const vec3_t pos);
	void		(*R_EntityParticles)		(struct cl_entity_s* ent);
	void		(*R_Explosion)				(const vec3_t pos, int model, float scale, float framerate, int flags);
	void		(*R_FizzEffect)				(struct cl_entity_s* pent, int modelIndex, int density);
	void		(*R_FireField) 				(const vec3_t org, int radius, int modelIndex, int count, int flags, float life);
	void		(*R_FlickerParticles)		(const vec3_t org);
	void		(*R_FunnelSprite)			(const vec3_t org, int modelIndex, int reverse);
	void		(*R_Implosion)				(const vec3_t end, float radius, int count, float life);
	void		(*R_LargeFunnel)			(const vec3_t org, int reverse);
	void		(*R_LavaSplash)				(const vec3_t org);
	void		(*R_MultiGunshot)			(const vec3_t org, const vec3_t dir, const vec3_t noise, int count, int decalCount, int* decalIndices);
	void		( *R_MuzzleFlash )			(int index, int iAttachment, const char *type);
	void		(*R_ParticleBox)			(const vec3_t mins, const vec3_t maxs, unsigned char r, unsigned char g, unsigned char b, float life);
	void		(*R_ParticleBurst)			(const vec3_t pos, int size, int color, float life);
	void		(*R_ParticleExplosion)		(const vec3_t org);
	void		(*R_ParticleExplosion2)		(const vec3_t org, int colorStart, int colorLength);
	void		(*R_ParticleLine)			(const vec3_t start, const vec3_t end, unsigned char r, unsigned char g, unsigned char b, float life);
	void		(*R_PlayerSprites)			(int client, int modelIndex, int count, int size);
	void		(*R_Projectile)				(const vec3_t origin, const vec3_t velocity, int modelIndex, int life, int owner, void (*hitcallback)(struct tempent_s* ent, struct pmtrace_s* ptr));
	void		(*R_RicochetSound)			(const vec3_t pos);
	void		(*R_RicochetSprite)			(const vec3_t pos, struct model_s* pmodel, float duration, float scale);
	void		(*R_RocketFlare)			(const vec3_t pos);
	void		(*R_RocketTrail)			(const vec3_t start, const vec3_t end, int type);
	void		(*R_RunParticleEffect)		(const vec3_t org, const vec3_t dir, int color, int count);
	void		(*R_ShowLine)				(const vec3_t start, const vec3_t end);
	void		(*R_SparkEffect)			(const vec3_t pos, int count, int velocityMin, int velocityMax);
	void		(*R_SparkShower)			(const vec3_t pos);
	void		(*R_SparkStreaks)			(const vec3_t pos, int count, int velocityMin, int velocityMax);
	void		(*R_Spray)					(const vec3_t pos, const vec3_t dir, int modelIndex, int count, int speed, int spread, int rendermode);
	void		(*R_Sprite_Explode)			(TEMPENTITY* pTemp, float scale, int flags);
	void		(*R_Sprite_Smoke)			(TEMPENTITY* pTemp, float scale);
	void		(*R_Sprite_Spray)			(const vec3_t pos, const vec3_t dir, int modelIndex, int count, int speed, int iRand);
	void		(*R_Sprite_Trail)			(int type, const vec3_t start, const vec3_t end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed);
	void		(*R_Sprite_TrailEx)			(int type, const vec3_t end, int modelIndex, int r, int g, int b, int count, float life, float size, int renderamt, int CanRoll, int ForceStop, int FadeOut, int CanBounce, float Gravity);
	void		(*R_Sprite_WallPuff)		(TEMPENTITY* pTemp, float scale);
	void		(*R_StreakSplash)			(const vec3_t pos, const vec3_t dir, int color, int count, float speed, int velocityMin, int velocityMax);
	void		(*R_TracerEffect)			(const vec3_t start, const vec3_t end);
	void		(*R_UserTracerParticle)		(const vec3_t org, const vec3_t vel, float life, int colorIndex, float length, unsigned char deathcontext, void (*deathfunc)(struct particle_s* particle));
	particle_t* (*R_TracerParticles)		(const vec3_t org, const vec3_t vel, float life);
	void		(*R_TeleportSplash)			(const vec3_t org);
	void		(*R_TempSphereModel)		(const vec3_t pos, float speed, float life, int count, int modelIndex);
	TEMPENTITY* (*R_TempModel)				(const vec3_t pos, const vec3_t dir, const vec3_t angles, float life, int modelIndex, int soundtype);
	TEMPENTITY* (*R_DefaultSprite)			(const vec3_t pos, int spriteIndex, float framerate);
	TEMPENTITY* (*R_TempSprite)				(const vec3_t pos, const vec3_t dir, float scale, int modelIndex, int rendermode, int renderfx, float a, float life, int flags);
	int			(*Draw_DecalIndex)			(int id);
	int			(*Draw_DecalIndexFromName)	(const char* name);
	void		(*R_DecalShoot)				(int textureIndex, int entity, int modelIndex, const vec3_t position, int flags);
	void		(*R_AttachTentToPlayer)		(int client, int modelIndex, float zoffset, float life);
	void		(*R_KillAttachedTents)		(int client);
	BEAM*		(*R_BeamCirclePoints)		(int type, const vec3_t start, const vec3_t end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b);
	BEAM*		(*R_BeamEntPoint)			(int startEnt, const vec3_t end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b);
	BEAM*		(*R_BeamEnts)				(int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b);
	BEAM*		(*R_BeamFollow)				(int startEnt, int modelIndex, float life, float width, float r, float g, float b, float brightness);
	void		(*R_BeamKill)				(int deadEntity);
	BEAM*		(*R_BeamLightning)			(const vec3_t start, const vec3_t end, int modelIndex, float life, float width, float amplitude, float brightness, float speed);
	BEAM*		(*R_BeamPoints)				(const vec3_t start, const vec3_t end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b);
	BEAM*		(*R_BeamRing)				(int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b);
	dlight_t*	(*CL_AllocDlight)			(int key);
	dlight_t*	(*CL_AllocElight)			(int key);
	TEMPENTITY* (*CL_TempEntAlloc)			(const vec3_t org, struct model_s* model);
	TEMPENTITY* (*CL_TempEntAllocNoModel)	(const vec3_t org);
	TEMPENTITY* (*CL_TempEntAllocHigh)		(const vec3_t org, struct model_s* model);
	TEMPENTITY* (*CL_TentEntAllocCustom)	(const vec3_t origin, struct model_s* model, int high, void (*callback) (struct tempent_s* ent, float frametime, float currenttime));
	void		(*R_GetPackedColor)			(short* packed, short color);
	short		(*R_LookupColor)			(unsigned char r, unsigned char g, unsigned char b);
	void		(*R_DecalRemoveAll)			(int textureIndex); //textureIndex points to the decal index in the array, not the actual texture index.
	void		(*R_FireCustomDecal)		(int textureIndex, int entity, int modelIndex, const vec3_t position, int flags, float scale);
	TEMPENTITY* (*R_TempCustomModel)			(const vec3_t pos, const vec3_t angles, const vec3_t velocity, float life, int modelIndex, int sequence, float framerate, bool fadeOut, int brightness, int rendermode, int entity, float fadeSpeed, bool fadeIn, float fadeInSpeed, float scale, int frameMax, int flags);
	BEAM*		(*R_BeamPoints_Stretch)			(const vec3_t start, const vec3_t end, int modelIndex, float life, float width, float brightness, int startFrame, float framerate, float r, float g, float b);
	void		(*R_KillAttachedTentsFromEntity)		(int client);
	TEMPENTITY* (*R_AttachTentToModel)			(int clientindex, int iAttachment, float life, int modelIndex, float framerate, int brightness, int rendermode, float scale, int frameMax, int flags, int eflags);
	BEAM* (*R_BeamPoints_Tracer)		(const vec3_t start, const vec3_t end, int modelIndex, float life, float width, float length, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b);
};

extern efx_api_t efx;

#endif
