#pragma once

#include "triangleapi.h"

// TriAPI
void TriRenderMode( int mode );
void TriBegin( int mode );
void TriEnd( void );
void TriColor4f( float r, float g, float b, float a );
void TriColor4ub( byte r, byte g, byte b, byte a );
void TriTexCoord2f( float u, float v );
void TriVertex3fv( const vec3_t v );
void TriVertex3f( float x, float y, float z );
void TriBrightness( float brightness );
void TriCullFace( TRICULLSTYLE mode );
int TriSpriteTexture( model_t *pSpriteModel, int frame );
int TriWorldToScreen( const vec3_t world, vec3_t_ref screen );
void TriFog( const vec3_t flFogColor, float flStart, float flEnd, int bOn );
void TriGetMatrix( const int pname, float *matrix );
void TriFogParams( float flDensity, int iFogSkybox );

void TriAPI_Init();
void TriAPI_Shutdown();