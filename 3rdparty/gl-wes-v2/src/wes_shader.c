/*
GL_MANGLE(gl-wes-v2:  OpenGL 2.0 to OGLESv2.0 wrapper
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "wes.h"
#include "wes_shader.h"
#include "wes_matrix.h"
#include "wes_fragment.h"

#define WES_PBUFFER_SIZE    128

char *wesShaderTestStr = "/*\n\
		gl-wes-v2:  OpenGL 2.0 to OGLESv2.0 wrapper\n\
		Contact:    lachlan.ts@gmail.com\n\
		Copyright (C) 2009  Lachlan Tychsen - Smith aka Adventus\n\
		\n\
		This library is free software; you can redistribute it and/or\n\
		modify it under the terms of the GNU Lesser General Public\n\
		License as published by the Free Software Foundation; either\n\
		version 3 of the License, or (at your option) any later version.\n\
		\n\
		This library is distributed in the hope that it will be useful,\n\
		but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n\
		Lesser General Public License for more details.\n\
		\n\
		You should have received a copy of the GNU Lesser General Public\n\
		License along with this library; if not, write to the Free\n\
		Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\
		*/\n\
		\n\
		#define LIGHT_NUM      8\n\
		#define CLIPPLANE_NUM     6\n\
		#define MULTITEX_NUM     4\n\
		#define FACE_NUM      2\n\
		\n\
		#define FACE_FRONT      0\n\
		#define FACE_BACK      1\n\
		\n\
		#define COLORMAT_AMBIENT    0\n\
		#define COLORMAT_DIFFUSE    1\n\
		#define COLORMAT_AMBIENT_AND_DIFFUSE 2\n\
		#define COLORMAT_SPECULAR    3\n\
		#define COLORMAT_EMISSIVE    4\n\
		\n\
		#define FUNC_NEVER                      0\n\
		#define FUNC_LESS                      1\n\
		#define FUNC_EQUAL                      2\n\
		#define FUNC_LEQUAL                     3\n\
		#define FUNC_GREATER                    4\n\
		#define FUNC_NOTEQUAL                   5\n\
		#define FUNC_GEQUAL                     6\n\
		#define FUNC_ALWAYS                     7\n\
		\n\
		#define FOG_LINEAR      0\n\
		#define FOG_EXP       1\n\
		#define FOG_EXP2      2\n\
		\n\
		#define GEN_OBJLINEAR     1\n\
		#define GEN_EYELINEAR     2\n\
		#define GEN_SPHEREMAP     3\n\
		#define GEN_REFLECTMAP     4\n\
		\n\
		struct sLight {\n\
		highp vec4  Position;\n\
		lowp vec4  ColorAmbient, ColorDiffuse, ColorSpec;\n\
		highp vec3  Attenuation;  // Constant, Linear & Quadratic factors\n\
		highp vec3 SpotDir;\n\
		highp vec2  SpotVar;  // Spot Shinniness & Cutoff angle\n\
		};\n\
		\n\
		struct sMaterial {\n\
		int   ColorMaterial;\n\
		lowp vec4  ColorAmbient, ColorDiffuse, ColorSpec, ColorEmissive;\n\
		float   SpecExponent;\n\
		};\n\
		\n\
		struct sLightModel {\n\
		lowp vec4  ColorAmbient;\n\
		bool  TwoSided;\n\
		bool  LocalViewer;\n\
		int   ColorControl;\n\
		};\n\
		\n\
		//Attributes:\n\
		attribute highp vec4  aPosition;\n\
		attribute mediump vec4  aTexCoord0;\n\
		attribute mediump vec4  aTexCoord1;\n\
		attribute mediump vec4  aTexCoord2;\n\
		attribute mediump vec4  aTexCoord3;\n\
		attribute highp vec3  aNormal;\n\
		attribute highp float  aFogCoord;\n\
		attribute lowp vec4  aColor;\n\
		attribute lowp vec4  aColor2nd;\n\
		\n\
		//Uniforms:\n\
		uniform bool   uEnableRescaleNormal;\n\
		uniform bool   uEnableNormalize;\n\
		\n\
		uniform highp mat4  uMVP;  //model-view-projection matrix\n\
		uniform highp mat4  uMV;  //model-view matrix\n\
		uniform highp mat3  uMVIT;  //model-view inverted & transformed matrix\n\
		\n\
		uniform bool   uEnableFog;\n\
		uniform bool   uEnableFogCoord;\n\
		uniform int    uFogMode;\n\
		uniform float   uFogDensity, uFogStart, uFogEnd;\n\
		\n\
		uniform bvec4   uEnableTextureGen[MULTITEX_NUM];\n\
		uniform ivec4   uTexGenMode[MULTITEX_NUM];\n\
		uniform mat4   uTexGenMat[MULTITEX_NUM];\n\
		\n\
		uniform bool   uEnableLighting;\n\
		uniform bool   uEnableLight[LIGHT_NUM];\n\
		uniform bool   uEnableColorMaterial;\n\
		uniform sLight   uLight[LIGHT_NUM];\n\
		uniform sLightModel  uLightModel;\n\
		uniform sMaterial  uMaterial[FACE_NUM];\n\
		uniform float    uRescaleFactor;\n\
		\n\
		uniform bool   uEnableClipPlane[CLIPPLANE_NUM];\n\
		uniform highp vec4   uClipPlane[CLIPPLANE_NUM];\n\
		\n\
		//Varyings:\n\
		varying lowp vec4   vColor;\n\
		varying lowp vec2  vFactor;\n\
		varying mediump vec4  vTexCoord[MULTITEX_NUM];\n\
		\n\
		\n\
		//local variables:\n\
		highp vec4    lEye;\n\
		highp vec3    lNormal;\n\
		lowp vec4    lMaterialAmbient;\n\
		lowp vec4    lMaterialDiffuse;\n\
		lowp vec4    lMaterialSpecular;\n\
		lowp vec4    lMaterialEmissive;\n\
		float     lMaterialSpecExponent;\n\
		int      lFace;\n\
		\n\
		void ComputeTexGen(){\n\
		for(int i = 0; i < MULTITEX_NUM; i++){\n\
		if (any(uEnableTextureGen[i])){\n\
		\n\
		if (uTexGenMode[i].x == GEN_OBJLINEAR){\n\
		vec4 row = vec4((uTexGenMat[i])[0][0], (uTexGenMat[i])[1][0], (uTexGenMat[i])[2][0], (uTexGenMat[i])[3][0]);\n\
		vTexCoord[i].x = dot(aPosition, row);\n\
		}\n\
		if (uTexGenMode[i].y == GEN_OBJLINEAR){\n\
		vec4 row = vec4((uTexGenMat[i])[0][1], (uTexGenMat[i])[1][1], (uTexGenMat[i])[2][1], (uTexGenMat[i])[3][1]);\n\
		vTexCoord[i].y = dot(aPosition, row);\n\
		}\n\
		if (uTexGenMode[i].z == GEN_OBJLINEAR){\n\
		vec4 row = vec4((uTexGenMat[i])[0][2], (uTexGenMat[i])[1][2], (uTexGenMat[i])[2][2], (uTexGenMat[i])[3][2]);\n\
		vTexCoord[i].z = dot(aPosition, row);\n\
		}\n\
		if (uTexGenMode[i].w == GEN_OBJLINEAR){\n\
		vec4 row = vec4((uTexGenMat[i])[0][3], (uTexGenMat[i])[1][3], (uTexGenMat[i])[2][3], (uTexGenMat[i])[3][3]);\n\
		vTexCoord[i].w = dot(aPosition, row);\n\
		}\n\
		\n\
		/* Must obtain full MVIT for these transforms*/\n\
		#if 0\n\
		if (uTexGenMode[i].x == GEN_OBJLINEAR){\n\
		vTexCoord[i].x = dot(lEye, (uTexGenMat[i])[0]);\n\
		}\n\
		if (uTexGenMode[i].y == GEN_OBJLINEAR){\n\
		vTexCoord[i].y = dot(lEye, (uTexGenMat[i])[1]);\n\
		}\n\
		if (uTexGenMode[i].z == GEN_OBJLINEAR){\n\
		vTexCoord[i].z = dot(lEye, (uTexGenMat[i])[2]);\n\
		}\n\
		if (uTexGenMode[i].w == GEN_OBJLINEAR){\n\
		vTexCoord[i].w = dot(lEye, (uTexGenMat[i])[3]);\n\
		}\n\
		#endif\n\
		\n\
		if (uTexGenMode[i].x == GEN_SPHEREMAP || uTexGenMode[i].y == GEN_SPHEREMAP ||\n\
		uTexGenMode[i].x == GEN_REFLECTMAP || uTexGenMode[i].y == GEN_REFLECTMAP ||\n\
		uTexGenMode[i].z == GEN_REFLECTMAP){\n\
		\n\
		vec3 u = normalize(lEye.xyz);\n\
		vec3 r = reflect(u, lNormal);\n\
		r.z += 1.0;\n\
		float m = 0.5 / length(r);\n\
		\n\
		if (uTexGenMode[i].x == GEN_SPHEREMAP)  {vTexCoord[i].x = (r.x * m) + 0.5;}\n\
		if (uTexGenMode[i].y == GEN_SPHEREMAP)  {vTexCoord[i].y = (r.y * m) + 0.5;}\n\
		if (uTexGenMode[i].x == GEN_REFLECTMAP)  {vTexCoord[i].x = r.x;}\n\
		if (uTexGenMode[i].y == GEN_REFLECTMAP)  {vTexCoord[i].y = r.y;}\n\
		if (uTexGenMode[i].z == GEN_REFLECTMAP)  {vTexCoord[i].z = r.z;}\n\
		}\n\
		}\n\
		}\n\
		\n\
		}\n\
		\n\
		void ComputeFog(float dist){\n\
		if(uFogMode == FOG_LINEAR){\n\
		vFactor.x = (uFogEnd - dist) / (uFogEnd - uFogStart);\n\
		} else if(uFogMode == FOG_EXP){\n\
		vFactor.x = exp(-(dist * uFogDensity));\n\
		} else {\n\
		vFactor.x = dist * uFogDensity;\n\
		vFactor.x = exp(-(vFactor.x * vFactor.x));\n\
		}\n\
		clamp(vFactor.x, 0.0, 1.0);\n\
		}\n\
		\n\
		vec4 ComputeLightFrom(int i){\n\
		highp vec3  dpos;\n\
		highp vec4 col;\n\
		float  dist, dist2, spot;\n\
		float  att;\n\
		float  ndoth, ndotl;\n\
		\n\
		dpos = uLight[i].Position.xyz;\n\
		att = 1.0;\n\
		\n\
		if (uLight[i].Position.w != 0.0){\n\
		dpos -= lEye.xyz;\n\
		\n\
		if (uLight[i].Attenuation.x != 1.0 || uLight[i].Attenuation.y != 0.0 ||\n\
		uLight[i].Attenuation.z != 0.0){\n\
		dist2 = dot(dpos, dpos);\n\
		dist = sqrt(dist2);\n\
		att = 1.0 / dot(uLight[i].Attenuation, vec3(1.0, dist, dist2));\n\
		}\n\
		\n\
		dpos = normalize(dpos);\n\
		if(uLight[i].SpotVar.y < 180.0){\n\
		spot = dot(-dpos, uLight[i].SpotDir);\n\
		if(spot >= cos(radians(uLight[i].SpotVar.y))){\n\
		att *= pow(spot, uLight[i].SpotVar.x);\n\
		} else {\n\
		return vec4(0,0,0,0);\n\
		}\n\
		}\n\
		}\n\
		\n\
		col = (uLight[i].ColorAmbient * lMaterialAmbient);\n\
		ndotl = dot(lNormal, dpos);\n\
		if (ndotl > 0.0){\n\
		col += ndotl * (uLight[i].ColorDiffuse * lMaterialDiffuse);\n\
		}\n\
		\n\
		dpos.z += 1.0;\n\
		dpos = normalize(dpos);\n\
		ndoth = dot(lNormal, dpos);\n\
		if (ndoth > 0.0){\n\
		col += pow(ndoth, lMaterialSpecExponent) * (lMaterialSpecular * uLight[i].ColorSpec);\n\
		}\n\
		\n\
		return att * col;\n\
		}\n\
		\n\
		void ComputeLighting(){\n\
		\n\
		/*  Determine Face  */\n\
		if (uLightModel.TwoSided){\n\
		if (lNormal.z > 0.0){\n\
		lFace = FACE_FRONT;\n\
		} else {\n\
		lNormal = -lNormal;\n\
		lFace = FACE_BACK;\n\
		}\n\
		} else {\n\
		lFace = FACE_FRONT;\n\
		}\n\
		\n\
		/* Determine which materials are to be used */\n\
		lMaterialAmbient = uMaterial[lFace].ColorAmbient;\n\
		lMaterialDiffuse = uMaterial[lFace].ColorDiffuse;\n\
		lMaterialSpecular = uMaterial[lFace].ColorSpec;\n\
		lMaterialEmissive = uMaterial[lFace].ColorEmissive;\n\
		lMaterialSpecExponent = uMaterial[lFace].SpecExponent;\n\
		if (uEnableColorMaterial){\n\
		if (uMaterial[lFace].ColorMaterial == COLORMAT_AMBIENT){\n\
		lMaterialAmbient = aColor;\n\
		} else if (uMaterial[lFace].ColorMaterial == COLORMAT_DIFFUSE){\n\
		lMaterialDiffuse = aColor;\n\
		} else if (uMaterial[lFace].ColorMaterial == COLORMAT_AMBIENT_AND_DIFFUSE){\n\
		lMaterialAmbient = aColor;\n\
		lMaterialDiffuse = aColor;\n\
		} else if (uMaterial[lFace].ColorMaterial == COLORMAT_SPECULAR){\n\
		lMaterialSpecular = aColor;\n\
		} else {\n\
		lMaterialEmissive =  aColor;\n\
		}\n\
		}\n\
		\n\
		vColor = lMaterialEmissive + lMaterialAmbient * uLightModel.ColorAmbient;\n\
		for(int i = 0; i < LIGHT_NUM; i++){\n\
		if (uEnableLight[i]){\n\
		vColor += ComputeLightFrom(i);\n\
		}\n\
		}\n\
		vColor.w = lMaterialDiffuse.w;\n\
		}\n\
		\n\
		void main(){\n\
		gl_Position = uMVP * aPosition;\n\
		lEye = uMV * aPosition;\n\
		lNormal = uMVIT * aNormal;\n\
		\n\
		if (uEnableRescaleNormal){\n\
		lNormal = uRescaleFactor * lNormal;\n\
		}\n\
		if (uEnableNormalize){\n\
		lNormal = normalize(lNormal);\n\
		}\n\
		\n\
		/* Lighting  */\n\
		if (uEnableLighting){\n\
		ComputeLighting();\n\
		} else {\n\
		vColor = aColor;\n\
		}\n\
		\n\
		/* Fog   */\n\
		vFactor.x = 1.0;\n\
		if (uEnableFog){\n\
		if (uEnableFogCoord){\n\
		ComputeFog(aFogCoord);\n\
		}else{\n\
		ComputeFog(-lEye.z);\n\
		}\n\
		}\n\
		\n\
		/* Tex Coord Generators  */\n\
		vTexCoord[0] = aTexCoord0;\n\
		vTexCoord[1] = aTexCoord1;\n\
		vTexCoord[2] = aTexCoord2;\n\
		vTexCoord[3] = aTexCoord3;\n\
		ComputeTexGen();\n\
		\n\
		/* Clip Planes */\n\
		vFactor.y = 1.0;\n\
		for(int i = 0; i < CLIPPLANE_NUM; i++){\n\
		if (uEnableClipPlane[i]){\n\
		if (dot(lEye, uClipPlane[i]) < 0.0){\n\
		vFactor.y = 0.0;\n\
		}\n\
		}\n\
		}\n\
		}\n\
";

GLboolean	sh_fallback;
char *wesShaderFallbackStr =
	"attribute highp vec4 	aPosition;\n\
	attribute lowp vec4 	aColor;\n\
	attribute mediump vec4 	aTexCoord0;\n\
	attribute mediump vec4 	aTexCoord1;\n\
	attribute mediump vec4 	aTexCoord2;\n\
	attribute mediump vec4 	aTexCoord3;\n\
	uniform highp mat4		uMVP;		//model-view-projection matrix\n\
	uniform highp mat4		uMV;		//model-view matrix\n\
	uniform highp mat3		uMVIT;		//model-view inverted & transformed matrix \n\
	uniform	bool			uEnableClipPlane[6];\n\
	uniform highp vec4 		uClipPlane[6];\n\
	\n\
	varying lowp vec4 		vColor;\n\
	varying lowp vec2		vFactor;\n\
	varying mediump vec4 	vTexCoord[4];\n\
	\n\
	highp vec4				lEye;\n\
	\n\
	void main(){\n\
		gl_Position = uMVP * aPosition;\n\
		lEye = uMV * aPosition;\n\
		vColor = aColor;\n\
		vTexCoord[0] = aTexCoord0;\n\
		vTexCoord[1] = aTexCoord1;\n\
		vTexCoord[2] = aTexCoord2;\n\
		vTexCoord[3] = aTexCoord3;\n\
		vFactor.x = 1.0;\n\
		vFactor.y = 1.0;\n\
	\n\
		for(int i = 0; i < 6; i++){\n\
			if (uEnableClipPlane[i]){\n\
				if (dot(lEye, uClipPlane[i]) < 0.0){\n\
					vFactor.y = 0.0;\n\
				}\n\
			}\n\
		}\n\
	}\n";




//shader global variables:
program_t       *sh_program;
GLboolean       sh_program_mod;
program_t       sh_pbuffer[WES_PBUFFER_SIZE];
GLuint          sh_pbuffer_count;
GLuint          sh_vertex;

//function declarations:
GLvoid
wes_shader_error(GLuint ind)
{
    int len;
    char* log;
    int i;
    wes_gl->glGetShaderiv(ind, GL_INFO_LOG_LENGTH, &len);
    log = (char*) malloc(len + 1);
    memset(log, 0, len+1);
    wes_gl->glGetShaderInfoLog(ind, len, &i, log);
    PRINT_ERROR("\nShader Error: %s", log);
    free(log);
}

GLvoid
wes_program_error(GLuint ind)
{
    int len;
    char* log;
    int i;
    wes_gl->glGetProgramiv(ind, GL_INFO_LOG_LENGTH, &len);
    log = (char*) malloc(len * sizeof(char));
    wes_gl->glGetProgramInfoLog(ind, len, &i, log);
    PRINT_ERROR("\nProgram Error: %s", log);
    free(log);
}

GLuint
wes_shader_create(char* data, GLenum type)
{
    GLuint  index;
    GLint   success;

    char *src[1];
    src[0] = data;

    LOGI("glCreateShader");
    //Compile:
    index = wes_gl->glCreateShader(type);
    LOGI("glCreateShader, index = %d\n", index);
    wes_gl->glShaderSource(index, 1, (const char**) src, NULL);
    LOGI("glShaderSource\n");
    
    wes_gl->glCompileShader(index);
    LOGI("glCompileShader\n");
  
    //test status:
    wes_gl->glGetShaderiv(index, GL_COMPILE_STATUS, &success);
	LOGI("glGetShaderiv\n");
    if (success){
	LOGI("shader success\n");
        return index;
    } else {
	LOGI("shader error\n");
        wes_shader_error(index);
        wes_gl->glDeleteShader(index);
        return (0xFFFFFFFF);
    }

}


GLvoid
wes_attrib_loc(GLuint prog)
{
    wes_gl->glBindAttribLocation(prog, WES_APOS,       "aPosition");
    wes_gl->glBindAttribLocation(prog, WES_ATEXCOORD0, "aTexCoord0");
    wes_gl->glBindAttribLocation(prog, WES_ATEXCOORD1, "aTexCoord1");
    wes_gl->glBindAttribLocation(prog, WES_ATEXCOORD2, "aTexCoord2");
    wes_gl->glBindAttribLocation(prog, WES_ATEXCOORD3, "aTexCoord3");
    wes_gl->glBindAttribLocation(prog, WES_ANORMAL,    "aNormal");
    wes_gl->glBindAttribLocation(prog, WES_AFOGCOORD,  "aFogCoord");
    wes_gl->glBindAttribLocation(prog, WES_ACOLOR0,    "aColor");
    wes_gl->glBindAttribLocation(prog, WES_ACOLOR1,    "aColor2nd");
}



GLvoid
wes_uniform_loc(program_t *p)
{
#define LocateUniform(A)                                                \
    p->uloc.A = wes_gl->glGetUniformLocation(p->prog, #A);
#define LocateUniformIndex(A, B, I)                                    \
    sprintf(str, #A "[%i]" #B, I);                                     \
    p->uloc.A[I]B = wes_gl->glGetUniformLocation(p->prog, str);

    int i;
    char str[256];

    LocateUniform(uEnableRescaleNormal);
    LocateUniform(uEnableNormalize);
    for(i = 0; i != WES_MULTITEX_NUM; i++)  {
        LocateUniformIndex(uEnableTextureGen, ,i);
    }
    for(i = 0; i != WES_CLIPPLANE_NUM; i++){
        LocateUniformIndex(uEnableClipPlane, ,i);
    }

    LocateUniform(uEnableFog);
    LocateUniform(uEnableFogCoord);
    LocateUniform(uEnableLighting);
    for(i = 0; i != WES_LIGHT_NUM; i++){
        LocateUniformIndex(uEnableLight, , i);
        LocateUniformIndex(uLight, .Position, i);
        LocateUniformIndex(uLight, .Attenuation, i);
        LocateUniformIndex(uLight, .ColorAmbient, i);
        LocateUniformIndex(uLight, .ColorDiffuse, i);
        LocateUniformIndex(uLight, .ColorSpec, i);
        LocateUniformIndex(uLight, .SpotDir, i);
        LocateUniformIndex(uLight, .SpotVar, i);
    }

    LocateUniform(uLightModel.ColorAmbient);
    LocateUniform(uLightModel.TwoSided);
    LocateUniform(uLightModel.LocalViewer);
    LocateUniform(uLightModel.ColorControl);
    LocateUniform(uRescaleFactor);

    for(i = 0; i < 2; i++){
        LocateUniformIndex(uMaterial, .ColorAmbient, i);
        LocateUniformIndex(uMaterial, .ColorDiffuse, i);
        LocateUniformIndex(uMaterial, .ColorSpec, i);
        LocateUniformIndex(uMaterial, .ColorEmissive, i);
        LocateUniformIndex(uMaterial, .SpecExponent, i);
        LocateUniformIndex(uMaterial, .ColorMaterial, i);
    }

    LocateUniform(uFogMode);
    LocateUniform(uFogDensity);
    LocateUniform(uFogStart);
    LocateUniform(uFogEnd);
    LocateUniform(uFogColor);

    for(i = 0; i != WES_CLIPPLANE_NUM; i++){
        LocateUniformIndex(uClipPlane, ,i);
    }

    LocateUniform(uMVP);
    LocateUniform(uMV);
    LocateUniform(uMVIT);
    LocateUniform(uAlphaRef);

    for(i = 0; i != WES_MULTITEX_NUM; i++){
        LocateUniformIndex(uTexUnit, , i);
        LocateUniformIndex(uTexEnvColor, , i);
    }

#undef LocateUniform
#undef LocateUniformIndex
}

GLuint
wes_program_create(GLuint frag, GLuint vert)
{
    GLuint  prog;
    GLint   success;

    //Create & attach
    prog = wes_gl->glCreateProgram();
    wes_gl->glAttachShader(prog, frag);
    wes_gl->glAttachShader(prog, vert);
    wes_gl->glLinkProgram(prog);

    //check status:
    wes_gl->glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!(success || wes_gl->glGetError())){
        wes_program_error(prog);
        wes_gl->glDeleteProgram(prog);
        return (0xFFFFFFFF);
    }

    wes_attrib_loc(prog);
    wes_gl->glLinkProgram(prog);

    //check status:
    wes_gl->glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (success || wes_gl->glGetError()){
        return prog;
    } else {
        wes_program_error(prog);
        wes_gl->glDeleteProgram(prog);
        return (0xFFFFFFFF);
    }
};

GLvoid
wes_build_program( progstate_t *s, program_t *p)
{
    char frag[4096];
    memset(frag, 0, 4096);
    wes_frag_build(frag, s);
    p->isbound = GL_FALSE;
    p->pstate = *s;
    p->vert = sh_vertex;
    p->frag = wes_shader_create(frag, GL_FRAGMENT_SHADER);
    p->prog = wes_program_create(p->frag, p->vert);
    wes_uniform_loc(p);
}

GLboolean
wes_progstate_cmp(progstate_t* s0, progstate_t* s1)
{
    GLint i, j;

    if (s0->uEnableAlphaTest != s1->uEnableAlphaTest)
        return 1;

    if (s0->uEnableAlphaTest && (s0->uAlphaFunc != s1->uAlphaFunc))
        return 1;

    if (s0->uEnableFog != s1->uEnableFog)
        return 1;

    if (s0->uEnableClipPlane != s1->uEnableClipPlane)
        return 1;

    for(i = 0; i != WES_MULTITEX_NUM; i++)
    {
		if (s0->uTexture[i].Enable != s1->uTexture[i].Enable)
			return 1;
		else if (s0->uTexture[i].Enable){

            if (s0->uTexture[i].Mode != s1->uTexture[i].Mode)
                return 1;

            if (s0->uTexture[i].Mode == WES_FUNC_COMBINE)
            {
                if (s0->uTexture[i].RGBCombine != s1->uTexture[i].RGBCombine)
                    return 1;
                if (s0->uTexture[i].AlphaCombine != s1->uTexture[i].AlphaCombine)
                    return 1;
				if (s0->uTexture[i].RGBScale != s1->uTexture[i].RGBScale)
					return 1;

                for(j = 0; j != 3; j++){
                    if (s0->uTexture[i].Arg[j].RGBSrc != s1->uTexture[i].Arg[j].RGBSrc)
                        return 1;
                    if (s0->uTexture[i].Arg[j].RGBOp != s1->uTexture[i].Arg[j].RGBOp)
                        return 1;
                    if (s0->uTexture[i].Arg[j].AlphaSrc != s1->uTexture[i].Arg[j].AlphaSrc)
                        return 1;
                    if (s0->uTexture[i].Arg[j].AlphaOp != s1->uTexture[i].Arg[j].AlphaOp)
                        return 1;
                    }
            }
        }
    }

    return 0;
}

GLvoid
wes_bind_program(program_t *p)
{
    if (p->isbound) return;
    if (sh_program) sh_program->isbound = GL_FALSE;
    sh_program_mod = GL_TRUE;
    sh_program = p;
    sh_program->isbound = GL_TRUE;
    wes_gl->glUseProgram(sh_program->prog);
}
//#define SHADER_FILE "WES.vsh"

GLvoid
wes_choose_program(progstate_t *s)
{
	unsigned int i, j;
    program_t *p;
    for(i = 0; i < sh_pbuffer_count; i++)
    {
        if (!wes_progstate_cmp(s, &sh_pbuffer[i].pstate))
        {
            if (sh_program != &sh_pbuffer[i])
            {
                p = &sh_pbuffer[i];
                wes_bind_program(p);
				/*
				fprintf(stdout, "===== Choose Fragment Shader =====\n");
				for(j = 0; j < WES_MULTITEX_NUM; j++){
					if (s->uTexture[j].Enable){
						fprintf(stdout, "TEX%i MODE %i \n", j, s->uTexture[j].Mode);
						fprintf(stdout, "TEX%i RGB COMB %i ALPHA COMB %i \n", j, s->uTexture[j].RGBCombine, s->uTexture[j].AlphaCombine);
						fprintf(stdout, "TEX%i RGB SRC = [%i, %i, %i] \n", j, s->uTexture[j].Arg[0].RGBSrc, s->uTexture[j].Arg[1].RGBSrc,s->uTexture[j].Arg[2].RGBSrc);
						fprintf(stdout, "TEX%i ALPHA SRC = [%i, %i, %i] \n", j, s->uTexture[j].Arg[0].AlphaSrc, s->uTexture[j].Arg[1].AlphaSrc,s->uTexture[j].Arg[2].AlphaSrc);
						fprintf(stdout, "TEX%i RGB OP = [%i, %i, %i] \n", j, s->uTexture[j].Arg[0].RGBOp, s->uTexture[j].Arg[1].RGBOp,s->uTexture[j].Arg[2].RGBOp);
						fprintf(stdout, "TEX%i ALPHA OP = [%i, %i, %i] \n", j, s->uTexture[j].Arg[0].AlphaOp, s->uTexture[j].Arg[1].AlphaOp,s->uTexture[j].Arg[2].AlphaOp);
					}
				}*/
            }
            return;
        }
    }

    p = &sh_pbuffer[sh_pbuffer_count];
    wes_build_program(s, p);
    wes_bind_program(p);
    sh_pbuffer_count++;

    if (sh_pbuffer_count == WES_PBUFFER_SIZE){
        PRINT_ERROR("Exceeded Maximum Programs!");
    }

}

GLvoid
wes_shader_init( void )
{
    FILE*           file;
    unsigned int    size;
    char*           data;

    sh_pbuffer_count = 0;
    sh_program_mod = GL_TRUE;


#ifdef SHADER_FILE
    //Load file into mem:
	file = fopen(SHADER_FILE, "rb");
	LOGI("Before shader load");
	if (!file){
	    LOGE("Could not find file: %s", "WES.vsh");
	}
 	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	data = (char*) malloc(size + 1);
	if (!data){
	    LOGE("Could not allocate: %i bytes", size + 1);
    }
	if (fread(data, sizeof(char), size, file) != size){
        free(data);
        LOGE("Could not read file: %s", "WES.vsh");
	}
	data[size] = '\0';
	fclose(file);

	LOGI("before shader create");
    sh_vertex = wes_shader_create(data, GL_VERTEX_SHADER);
	LOGI("after shader create");
    free(data);
#else
#define SHADER_COMPUTE_LIGHTING
#ifdef SHADER_COMPUTE_LIGHTING
	//data = (char*) malloc(wesShaderLightingStr.size() + 1);
	//strcpy(data, wesShaderLightingStr.c_str());
	
	LOGI("Before shader load");
	data = (char*) malloc(strlen(wesShaderTestStr) + 1);
	LOGI("malloc shader load");
	strcpy(data, wesShaderTestStr);
	LOGI("strcpy shader load");
#else
	data = (char*) malloc(wesShaderStr.size() + 1);
	strcpy(data, wesShaderStr.c_str());
#endif
	//LOGI("Shader = %s", data);
	sh_vertex = wes_shader_create(data, GL_VERTEX_SHADER);
	if(sh_vertex == 0xFFFFFFFF)
	{
		free(data);
		data = (char*) malloc(strlen(wesShaderFallbackStr) + 1);
		strcpy(data, wesShaderFallbackStr);
		sh_vertex = wes_shader_create(data, GL_VERTEX_SHADER);
		sh_fallback = GL_TRUE;
	}

	LOGI("after shader create");
	free(data);
	LOGI("free");
#endif
}


GLvoid
wes_shader_destroy( void )
{
    unsigned int i;
    wes_gl->glDeleteShader(sh_vertex);
    for(i = 0; i < sh_pbuffer_count; i++)
    {
        wes_gl->glDeleteShader(sh_pbuffer[i].frag);
        wes_gl->glDeleteProgram(sh_pbuffer[i].prog);
    }
}



