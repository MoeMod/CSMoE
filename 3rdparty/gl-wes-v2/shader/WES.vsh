/*
gl-wes-v2:  OpenGL 2.0 to OGLESv2.0 wrapper
Contact:    lachlan.ts@gmail.com
Copyright (C) 2009  Lachlan Tychsen - Smith aka Adventus

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define LIGHT_NUM						8
#define CLIPPLANE_NUM					6
#define MULTITEX_NUM					4
#define FACE_NUM						2

#define FACE_FRONT						0
#define FACE_BACK						1

#define COLORMAT_AMBIENT				0
#define COLORMAT_DIFFUSE				1
#define COLORMAT_AMBIENT_AND_DIFFUSE	2
#define COLORMAT_SPECULAR				3
#define COLORMAT_EMISSIVE				4

#define FUNC_NEVER                     	0
#define FUNC_LESS                     	1
#define FUNC_EQUAL                      2
#define FUNC_LEQUAL                     3
#define FUNC_GREATER                    4
#define FUNC_NOTEQUAL                   5
#define FUNC_GEQUAL                     6
#define FUNC_ALWAYS                     7

#define FOG_LINEAR						0
#define FOG_EXP							1
#define FOG_EXP2						2

#define GEN_OBJLINEAR					1
#define GEN_EYELINEAR					2
#define GEN_SPHEREMAP					3
#define GEN_REFLECTMAP					4

struct sLight {
	highp vec4 	Position;
	lowp vec4 	ColorAmbient, ColorDiffuse, ColorSpec;
	highp vec3 	Attenuation; 	// Constant, Linear & Quadratic factors
	highp vec3	SpotDir;
	highp vec2 	SpotVar;		// Spot Shinniness & Cutoff angle
};

struct sMaterial {
	int 		ColorMaterial;
	lowp vec4 	ColorAmbient, ColorDiffuse, ColorSpec, ColorEmissive;
	float 		SpecExponent;
};

struct sLightModel {
	lowp vec4 	ColorAmbient;
	bool		TwoSided;	
	bool		LocalViewer;
	int			ColorControl;		
};

//Attributes:
attribute highp vec4 	aPosition;
attribute mediump vec4 	aTexCoord0;
attribute mediump vec4 	aTexCoord1;
attribute mediump vec4 	aTexCoord2;
attribute mediump vec4 	aTexCoord3;
attribute highp vec3 	aNormal;
attribute highp float 	aFogCoord;
attribute lowp vec4 	aColor;
attribute lowp vec4 	aColor2nd;

//Uniforms:
uniform	bool			uEnableRescaleNormal;
uniform	bool			uEnableNormalize;

uniform highp mat4		uMVP;		//model-view-projection matrix
uniform highp mat4		uMV;		//model-view matrix
uniform highp mat3		uMVIT;		//model-view inverted & transformed matrix 

uniform	bool			uEnableFog;
uniform	bool			uEnableFogCoord;
uniform int				uFogMode;
uniform float			uFogDensity, uFogStart, uFogEnd;

uniform bvec4			uEnableTextureGen[MULTITEX_NUM];
uniform ivec4			uTexGenMode[MULTITEX_NUM];
uniform mat4			uTexGenMat[MULTITEX_NUM];

uniform	bool			uEnableLighting;
uniform	bool			uEnableLight[LIGHT_NUM];
uniform	bool			uEnableColorMaterial;
uniform sLight			uLight[LIGHT_NUM];
uniform sLightModel		uLightModel;
uniform sMaterial		uMaterial[FACE_NUM];
uniform float 			uRescaleFactor;

uniform	bool			uEnableClipPlane[CLIPPLANE_NUM];
uniform highp vec4 		uClipPlane[CLIPPLANE_NUM];

//Varyings:
varying lowp vec4 		vColor;
varying lowp vec2		vFactor;
varying mediump vec4 	vTexCoord[MULTITEX_NUM];


//local variables:
highp vec4				lEye;
highp vec3				lNormal;
lowp vec4				lMaterialAmbient;
lowp vec4				lMaterialDiffuse;
lowp vec4				lMaterialSpecular;
lowp vec4				lMaterialEmissive;
float					lMaterialSpecExponent;
int						lFace;

void ComputeTexGen(int i){

	if (uTexGenMode[i].x == GEN_OBJLINEAR){
		vec4 row = vec4((uTexGenMat[i])[0][0], (uTexGenMat[i])[1][0], (uTexGenMat[i])[2][0], (uTexGenMat[i])[3][0]);
		vTexCoord[i].x = dot(aPosition, row);
		}
	if (uTexGenMode[i].y == GEN_OBJLINEAR){
		vec4 row = vec4((uTexGenMat[i])[0][1], (uTexGenMat[i])[1][1], (uTexGenMat[i])[2][1], (uTexGenMat[i])[3][1]);
		vTexCoord[i].y = dot(aPosition, row);
	}
	if (uTexGenMode[i].z == GEN_OBJLINEAR){
		vec4 row = vec4((uTexGenMat[i])[0][2], (uTexGenMat[i])[1][2], (uTexGenMat[i])[2][2], (uTexGenMat[i])[3][2]);
		vTexCoord[i].z = dot(aPosition, row);
	}
	if (uTexGenMode[i].w == GEN_OBJLINEAR){
		vec4 row = vec4((uTexGenMat[i])[0][3], (uTexGenMat[i])[1][3], (uTexGenMat[i])[2][3], (uTexGenMat[i])[3][3]);
		vTexCoord[i].w = dot(aPosition, row);
	}

	/* Must obtain full MVIT for these transforms*/
#if 0
	if (uTexGenMode[i].x == GEN_OBJLINEAR){
		vTexCoord[i].x = dot(lEye, (uTexGenMat[i])[0]);
	}
	if (uTexGenMode[i].y == GEN_OBJLINEAR){
		vTexCoord[i].y = dot(lEye, (uTexGenMat[i])[1]);
	}
	if (uTexGenMode[i].z == GEN_OBJLINEAR){
		vTexCoord[i].z = dot(lEye, (uTexGenMat[i])[2]);
	}
	if (uTexGenMode[i].w == GEN_OBJLINEAR){
		vTexCoord[i].w = dot(lEye, (uTexGenMat[i])[3]);
	}
#endif
	
	if (uTexGenMode[i].x == GEN_SPHEREMAP || uTexGenMode[i].y == GEN_SPHEREMAP || 
		uTexGenMode[i].x == GEN_REFLECTMAP || uTexGenMode[i].y == GEN_REFLECTMAP ||
		uTexGenMode[i].z == GEN_REFLECTMAP){
		
		vec3 u = normalize(lEye.xyz);
		vec3 r = reflect(u, lNormal);
		r.z += 1.0;
		float m = 0.5 / length(r);
		
		if (uTexGenMode[i].x == GEN_SPHEREMAP)		{vTexCoord[i].x = (r.x * m) + 0.5;}
		if (uTexGenMode[i].y == GEN_SPHEREMAP)		{vTexCoord[i].y = (r.y * m) + 0.5;}
		if (uTexGenMode[i].x == GEN_REFLECTMAP)		{vTexCoord[i].x = r.x;}
		if (uTexGenMode[i].y == GEN_REFLECTMAP)		{vTexCoord[i].y = r.y;}
		if (uTexGenMode[i].z == GEN_REFLECTMAP)		{vTexCoord[i].z = r.z;}
	}
}

void ComputeFog(float dist){
	if(uFogMode == FOG_LINEAR){
		vFactor.x = (uFogEnd - dist) / (uFogEnd - uFogStart);
	} else if(uFogMode == FOG_EXP){
		vFactor.x = exp(-(dist * uFogDensity));
	} else {
		vFactor.x = dist * uFogDensity;
		vFactor.x = exp(-(vFactor.x * vFactor.x));
	}
	clamp(vFactor.x, 0.0, 1.0);
}

vec4 ComputeLightFrom(int i){
	highp vec3 	dpos;
	highp vec4	col;
	float 	dist, dist2, spot;
	float 	att;
	float 	ndoth, ndotl;
	
	dpos = uLight[i].Position.xyz;
	att = 1.0;	

	if (uLight[i].Position.w != 0.0){
		dpos -= lEye.xyz;
		
		if (uLight[i].Attenuation.x != 1.0 || uLight[i].Attenuation.y != 0.0 ||
			uLight[i].Attenuation.z != 0.0){
			dist2 = dot(dpos, dpos);
			dist = sqrt(dist2);
			att = 1.0 / dot(uLight[i].Attenuation, vec3(1.0, dist, dist2));
		}
		
		dpos = normalize(dpos);
		if(uLight[i].SpotVar.y < 180.0){
			spot = dot(-dpos, uLight[i].SpotDir);
			if(spot >= cos(radians(uLight[i].SpotVar.y))){
				att *= pow(spot, uLight[i].SpotVar.x);
			} else {
				return vec4(0,0,0,0);
			}
		}
	}
	
	col = (uLight[i].ColorAmbient * lMaterialAmbient);
	ndotl = dot(lNormal, dpos);
	if (ndotl > 0.0){
		col += ndotl * (uLight[i].ColorDiffuse * lMaterialDiffuse);
	}
	
	dpos.z += 1.0;
	dpos = normalize(dpos);
	ndoth = dot(lNormal, dpos);
	if (ndoth > 0.0){ 
		col += pow(ndoth, lMaterialSpecExponent) * (lMaterialSpecular * uLight[i].ColorSpec);
	}
	
	return att * col;
}

void ComputeLighting(){

	/* 	Determine Face 	*/
	if (uLightModel.TwoSided){
		if (lNormal.z > 0.0){
			lFace = FACE_FRONT;
		} else {
			lNormal = -lNormal;
			lFace = FACE_BACK;
		}	
	} else {
		lFace = FACE_FRONT;
	}
		
	/* Determine which materials are to be used	*/
	lMaterialAmbient = uMaterial[lFace].ColorAmbient;
	lMaterialDiffuse = uMaterial[lFace].ColorDiffuse;
	lMaterialSpecular = uMaterial[lFace].ColorSpec;
	lMaterialEmissive = uMaterial[lFace].ColorEmissive;
	lMaterialSpecExponent = uMaterial[lFace].SpecExponent;
	if (uEnableColorMaterial){
		if (uMaterial[lFace].ColorMaterial == COLORMAT_AMBIENT){
			lMaterialAmbient = aColor;
		} else if (uMaterial[lFace].ColorMaterial == COLORMAT_DIFFUSE){
			lMaterialDiffuse = aColor;
		} else if (uMaterial[lFace].ColorMaterial == COLORMAT_AMBIENT_AND_DIFFUSE){
			lMaterialAmbient = aColor;
			lMaterialDiffuse = aColor;
		} else if (uMaterial[lFace].ColorMaterial == COLORMAT_SPECULAR){
			lMaterialSpecular = aColor;
		} else {
			lMaterialEmissive =  aColor;
		}
	}
		
	vColor = lMaterialEmissive + lMaterialAmbient * uLightModel.ColorAmbient;
	for(int i = 0; i < LIGHT_NUM; i++){
		if (uEnableLight[i]){
			vColor += ComputeLightFrom(i);
		}
	}
	vColor.w = lMaterialDiffuse.w;
}

void main(){
	gl_Position = uMVP * aPosition;
	lEye = uMV * aPosition;
	lNormal = uMVIT * aNormal;
		
	if (uEnableRescaleNormal){
		lNormal = uRescaleFactor * lNormal;
	}
	if (uEnableNormalize){
		lNormal = normalize(lNormal);
	}
	
	/* Lighting 	*/
	if (uEnableLighting){
		ComputeLighting();
	} else {
		vColor = aColor;
	}
	
	/* Fog			*/
	vFactor.x = 1.0;
	if (uEnableFog){
		if (uEnableFogCoord){
			ComputeFog(aFogCoord);			
		}else{
			ComputeFog(-lEye.z);
		}	
	}	

	/* Tex Coord Generators 	*/
	vTexCoord[0] = aTexCoord0;
	vTexCoord[1] = aTexCoord1;
	vTexCoord[2] = aTexCoord2;
	vTexCoord[3] = aTexCoord3;
	for(int i = 0; i < MULTITEX_NUM; i++){
		if (any(uEnableTextureGen[i])){
			ComputeTexGen(i);
		}
	}

	/* Clip Planes	*/
	vFactor.y = 1.0;
	for(int i = 0; i < CLIPPLANE_NUM; i++){
		if (uEnableClipPlane[i]){
			if (dot(lEye, uClipPlane[i]) < 0.0){
				vFactor.y = 0.0;
			}
		}
	}
}