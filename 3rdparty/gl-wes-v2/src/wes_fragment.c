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
#include "wes_state.h"
#include "wes_shader.h"
#include "wes.h"

#define max(A, B)   ((A > B) ? A : B)

const char* frag_header = ""
//						  "#define highp\n#define lowp\n#define mediump                         \n  "
"#define LIGHT_NUM						8           \n  "
"#define CLIPPLANE_NUM					6           \n  "
"#define MULTITEX_NUM					4           \n  "
"#define FACE_NUM						2           \n  "
"                                                   \n  "
"uniform sampler2D	    uTexUnit[MULTITEX_NUM];     \n  "
"uniform lowp vec4	    uTexEnvColor[MULTITEX_NUM]; \n  "
"uniform lowp vec4		uFogColor;                  \n  "
"uniform highp float	uAlphaRef;                  \n  "
"                                                   \n  "
"//Varyings:                                        \n  "
"varying lowp vec4 		vColor;                     \n  "
"varying lowp vec2 		vFactor;                    \n  "
"varying mediump vec4 	vTexCoord[MULTITEX_NUM];    \n  "
"                                                   \n  "
"void main(){                                       \n  "
"    gl_FragColor = vColor;                         \n  ";


GLint
wes_frag_clipplane(char* buff, progstate_t *s)
{
    char *str = buff;

    if (s->uEnableClipPlane){
        str += sprintf(str,"%s\n", "gl_FragColor.a *= vFactor.y;");
    }

    return (GLint)(str - buff);
};

GLint
wes_frag_fog(char* buff, progstate_t *s)
{
    char *str = buff;
	if (s->uEnableFog)
    {
        str += sprintf(str,"gl_FragColor = mix(uFogColor, gl_FragColor, vFactor.x);\n");
        //gl_FragColor * vFactor.x + (1.0 - vFactor.x) * uFogColor;");
    }
    return (GLint)(str - buff);
};

GLint
wes_frag_alphatest(char* buff, progstate_t *s)
{
    char *str = buff;
    if (s->uEnableAlphaTest){
        switch(s->uAlphaFunc){
            case WES_ALPHA_NEVER:
                str += sprintf(str, "%s\n", "discard;");
                break;

            case WES_ALPHA_LESS:
                str += sprintf(str, "%s\n", "if (gl_FragColor.w >= uAlphaRef) discard;");
                break;

            case WES_ALPHA_EQUAL:
                str += sprintf(str, "%s\n", "if (gl_FragColor.w != uAlphaRef) discard;");
                break;

            case WES_ALPHA_LEQUAL:
                str += sprintf(str, "%s\n", "if (gl_FragColor.w > uAlphaRef) discard;");
                break;

            case WES_ALPHA_GREATER:
                str += sprintf(str, "%s\n", "if (gl_FragColor.w <= uAlphaRef) discard;");
                break;

            case WES_ALPHA_NOTEQUAL:
                str += sprintf(str, "%s\n", "if (gl_FragColor.w == uAlphaRef) discard;");
                break;

            case WES_ALPHA_GEQUAL:
                str += sprintf(str, "%s\n", "if (gl_FragColor.w < uAlphaRef) discard;");
                break;

        }
    }
    return (GLint)(str - buff);
};


GLint
wes_frag_op(char *buff, progstate_t *s, int tex, int arg)
{
    char *str = buff;

    switch(s->uTexture[tex].Arg[arg].RGBOp)
    {
        case WES_OP_ALPHA:
            str += sprintf(str, "arg%i.rgb = vec3(arg%i.a);\n", arg, arg);break;
        case WES_OP_ONE_MINUS_ALPHA:
            str += sprintf(str, "arg%i.rgb = vec3(1.0 - arg%i.a);\n", arg, arg);break;
        case WES_OP_COLOR:
            break;
        case WES_OP_ONE_MINUS_COLOR:
            str += sprintf(str, "arg%i.rgb = vec3(1.0) - arg%i.rgb;\n", arg, arg);break;
    }

    switch(s->uTexture[tex].Arg[arg].AlphaOp)
    {
        case WES_OP_ALPHA:
            break;
        case WES_OP_ONE_MINUS_ALPHA:
            str += sprintf(str, "arg%i.a = 1.0 - arg%i.a;\n", arg, arg);break;
    }

    return (GLint)(str - buff);

}

GLint
wes_frag_arg_obtain(char *buff, progstate_t *s, int tex, int arg, int type)
{
    char *str = buff;

    int src = 0;
    char comp[5];
    switch(type){
        case 4: strcpy(comp, "rgba"); src = s->uTexture[tex].Arg[arg].RGBSrc;    break;
        case 3: strcpy(comp, "rgb");  src = s->uTexture[tex].Arg[arg].RGBSrc;    break;
        case 1: strcpy(comp, "a");    src = s->uTexture[tex].Arg[arg].AlphaSrc;  break;
        default: PRINT_ERROR("Unknown Argument Type: %i", type); break;
    }

    switch(src)
    {
        case WES_SRC_PREVIOUS:
            if (tex == 0){
                str += sprintf(str, "arg%i.%s = vColor.%s;\n", arg, comp, comp);
            } else {
                str += sprintf(str, "arg%i.%s = gl_FragColor.%s;\n", arg, comp, comp);
            }
            break;
        case WES_SRC_CONSTANT:
            str += sprintf(str, "arg%i.%s = uTexEnvColor[%i].%s;\n", arg, comp, tex, comp);
            break;
        case WES_SRC_PRIMARY_COLOR:
            str += sprintf(str, "arg%i.%s = vColor.%s;\n", arg, comp, comp);
            break;
        case WES_SRC_TEXTURE:
            str += sprintf(str, "arg%i.%s = tex%i.%s;\n", arg, comp, tex, comp);
            break;

        /* Crossbar Extensions */
        case WES_SRC_TEXTURE0:
            str += sprintf(str, "arg%i.%s = tex%i.%s;\n", arg, comp, 0, comp);
            break;
        case WES_SRC_TEXTURE1:
            str += sprintf(str, "arg%i.%s = tex%i.%s;\n", arg, comp, 1, comp);
            break;
        case WES_SRC_TEXTURE2:
            str += sprintf(str, "arg%i.%s = tex%i.%s;\n", arg, comp, 2, comp);
            break;
        case WES_SRC_TEXTURE3:
            str += sprintf(str, "arg%i.%s = tex%i.%s;\n", arg, comp, 3, comp);
            break;

        /* ATI Extensions */
        case WES_SRC_ONE:
            if (type == 1)
                str += sprintf(str, "arg%i.%s = 1.0;\n", arg, comp);
            else
                str += sprintf(str, "arg%i.%s = vec%i(1.0);\n", arg, comp, type);
            break;
        case WES_SRC_ZERO:
            if (type == 1)
                str += sprintf(str, "arg%i.%s = 0.0;\n", arg, comp);
            else
                str += sprintf(str, "arg%i.%s = vec%i(0.0);\n", arg, comp, type);
            break;

        default:    PRINT_ERROR("ERROR: No valid SRC for TEX%i ARG%i.%s", tex, arg, comp); break;
    }
    return (GLint)(str - buff);
};

GLvoid
wes_frag_nargs(progstate_t *s, GLint tex, GLint *narg_rgb, GLint *narg_alpha)
{
    *narg_rgb = *narg_alpha = 0;
    switch(s->uTexture[tex].RGBCombine){
        case WES_FUNC_REPLACE:
            *narg_rgb = max(1, *narg_rgb);
            break;

        case WES_FUNC_MODULATE:
        case WES_FUNC_ADD:
        case WES_FUNC_ADD_SIGNED:
        case WES_FUNC_SUBTRACT:
        case WES_FUNC_DOT3_RGBA:
        case WES_FUNC_DOT3_RGB:
            *narg_rgb = max(2, *narg_rgb);
            break;

        case WES_FUNC_INTERPOLATE:
        case WES_FUNC_MODULATE_SUBTRACT:
        case WES_FUNC_MODULATE_ADD:
        case WES_FUNC_MODULATE_SIGNED_ADD:
            *narg_rgb = max(3, *narg_rgb);
            break;

        default: PRINT_ERROR("Could not determine number of neccessary arguments for Texture Unit %i", tex);
    }

    if (s->uTexture[tex].RGBCombine != WES_FUNC_DOT3_RGBA){
        switch(s->uTexture[tex].AlphaCombine){
            case WES_FUNC_REPLACE:
                *narg_alpha = max(1, *narg_alpha);
                break;

            case WES_FUNC_MODULATE:
            case WES_FUNC_ADD:
            case WES_FUNC_ADD_SIGNED:
            case WES_FUNC_SUBTRACT:
                *narg_alpha = max(2, *narg_alpha);
                break;

            case WES_FUNC_INTERPOLATE:
            case WES_FUNC_MODULATE_SUBTRACT:
            case WES_FUNC_MODULATE_ADD:
            case WES_FUNC_MODULATE_SIGNED_ADD:
                *narg_alpha = max(3, *narg_alpha);
                break;

            default: PRINT_ERROR("Could not determine number of neccessary arguments for Texture Unit %i", tex);
        }
    }
}

GLint
wes_frag_args(char *buff, progstate_t *s, int tex)
{
    char *str = buff;
    GLint arg, rgbsrc, alphasrc, rgbop, alphaop;
    GLint narg = 0, narg_rgb = 0, narg_alpha = 0;

    wes_frag_nargs(s, tex, &narg_rgb, &narg_alpha);
    narg = max(narg_alpha, narg_rgb);
    for(arg = 0; arg < narg; arg++){
        rgbop = s->uTexture[tex].Arg[arg].RGBOp;
        alphaop = s->uTexture[tex].Arg[arg].AlphaOp;
        rgbsrc = s->uTexture[tex].Arg[arg].RGBSrc;
        alphasrc = s->uTexture[tex].Arg[arg].AlphaSrc;

        /* obtain non duplicate components */
        if (rgbsrc == alphasrc){
            str += wes_frag_arg_obtain(str, s, tex, arg, 4);
        } else {
            if (!(arg >= narg_rgb && alphaop != WES_OP_COLOR && alphaop != WES_OP_ONE_MINUS_COLOR)
                && (rgbop != WES_OP_ALPHA && rgbop != WES_OP_ONE_MINUS_ALPHA)){
                    str += wes_frag_arg_obtain(str, s, tex, arg, 3);
            }
            if (!(arg >= narg_alpha && rgbop != WES_OP_ALPHA && rgbop != WES_OP_ONE_MINUS_ALPHA)){
                str += wes_frag_arg_obtain(str, s, tex, arg, 1);
            }
        }
    }

    for(arg = 0; arg < narg; arg++){
        str += wes_frag_op(str, s, tex, arg);
    }

    return (GLint)(str - buff);
}

GLint
wes_frag_combine(char *buff, progstate_t *s, int tex)
{
    char *str  = buff;

    str += wes_frag_args(str, s, tex);
    if (s->uTexture[tex].RGBCombine == s->uTexture[tex].AlphaCombine)
    {
        switch(s->uTexture[tex].RGBCombine)
        {
            case WES_FUNC_REPLACE:
                str += sprintf(str, "gl_FragColor = arg0;\n");
                break;

            case WES_FUNC_MODULATE:
                str += sprintf(str, "gl_FragColor = arg0 * arg1;\n");
                break;

            case WES_FUNC_ADD:
                str += sprintf(str, "gl_FragColor = arg0 + arg1;\n");
                break;

            case WES_FUNC_ADD_SIGNED:
                str += sprintf(str, "gl_FragColor = arg0 + arg1 - vec4(0.5);\n");
                break;

            case WES_FUNC_INTERPOLATE:
                str += sprintf(str, "gl_FragColor = mix(arg1, arg0, arg2);\n");
                break;

            case WES_FUNC_SUBTRACT:
                str += sprintf(str, "gl_FragColor = arg0 - arg1;\n");
                break;

            /*ATI Extensions */
            case WES_FUNC_MODULATE_SUBTRACT:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg2.rgb - arg1.rgb;\n");
                break;

            case WES_FUNC_MODULATE_ADD:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg2.rgb + arg1.rgb;\n");
                break;

            case WES_FUNC_MODULATE_SIGNED_ADD:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg2.rgb + arg1.rgb - vec3(0.5);\n");
                break;

            default:    PRINT_ERROR("ERROR: No valid FUNC for TEX%i", tex); break;

        }

    } else if (s->uTexture[tex].RGBCombine == WES_FUNC_DOT3_RGBA){
        str += sprintf(str, "lowp vec3 col0 = arg0.rgb - vec3(0.5);\n");
        str += sprintf(str, "lowp vec3 col1 = arg1.rgb - vec3(0.5);\n");
        str += sprintf(str, "gl_FragColor = vec4(4.0 * dot(col0, col1));\n");
    } else {

        switch(s->uTexture[tex].RGBCombine)
        {
            case WES_FUNC_REPLACE:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb;\n");
                break;

            case WES_FUNC_MODULATE:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg1.rgb;\n");
                break;

            case WES_FUNC_ADD:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb + arg1.rgb;\n");
                break;

            case WES_FUNC_ADD_SIGNED:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb + arg1.rgb - vec3(0.5);\n");
                break;

            case WES_FUNC_INTERPOLATE:
                str += sprintf(str, "gl_FragColor.rgb = mix(arg1.rgb, arg0.rgb, arg2.rgb);\n");
                break;

            case WES_FUNC_SUBTRACT:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb - arg1.rgb;\n");
                break;

            case WES_FUNC_DOT3_RGB:
				str += sprintf(str, "lowp vec3 col0 = arg0.rgb - vec3(0.5);\n");
				str += sprintf(str, "lowp vec3 col1 = arg1.rgb - vec3(0.5);\n");
				str += sprintf(str, "gl_FragColor.rgb = vec3(4.0 * dot(col0, col1));\n");
                break;

            /*ATI Extensions */
            case WES_FUNC_MODULATE_SUBTRACT:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg2.rgb - arg1.rgb;\n");
                break;

            case WES_FUNC_MODULATE_ADD:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg2.rgb + arg1.rgb;\n");
                break;

            case WES_FUNC_MODULATE_SIGNED_ADD:
                str += sprintf(str, "gl_FragColor.rgb = arg0.rgb * arg2.rgb + arg1.rgb - vec3(0.5);\n");
                break;

            default:    PRINT_ERROR("ERROR: No valid FUNC for TEX%i.rgb", tex); break;
        }

        switch(s->uTexture[tex].AlphaCombine)
        {
            case WES_FUNC_REPLACE:
                str += sprintf(str, "gl_FragColor.a = arg0.a;\n");
                break;

            case WES_FUNC_MODULATE:
                str += sprintf(str, "gl_FragColor.a = arg0.a * arg1.a;\n");
                break;

            case WES_FUNC_ADD:
                str += sprintf(str, "gl_FragColor.a = arg0.a + arg1.a;\n");
                break;

            case WES_FUNC_ADD_SIGNED:
                str += sprintf(str, "gl_FragColor.a = arg0.a + arg1.a - 0.5;\n");
                break;

            case WES_FUNC_INTERPOLATE:
                str += sprintf(str, "gl_FragColor.a = mix(arg1.a, arg0.a, arg2.a);\n");
                break;

            case WES_FUNC_SUBTRACT:
                str += sprintf(str, "gl_FragColor.a = arg0.a - arg1.a;\n");
                break;

            /*ATI Extensions */
            case WES_FUNC_MODULATE_SUBTRACT:
                str += sprintf(str, "gl_FragColor.a = arg0.a * arg2.a - arg1.a;\n");
                break;

            case WES_FUNC_MODULATE_ADD:
                str += sprintf(str, "gl_FragColor.a = arg0.a * arg2.a + arg1.a;\n");
                break;

            case WES_FUNC_MODULATE_SIGNED_ADD:
                str += sprintf(str, "gl_FragColor.a = arg0.a * arg2.a + arg1.a - 0.5;\n");
                break;

            default:    PRINT_ERROR("ERROR: No valid FUNC for TEX%i.a", tex); break;
        }

    }
	if(s->uTexture[tex].RGBScale)
		str += sprintf(str, "gl_FragColor.rgb *= %f;\n", s->uTexture[tex].RGBScale );

    return (GLint)(str - buff);
}

GLint
wes_frag_tex_obtain(char* buff, progstate_t *s){
    char *str = buff;
    GLint i, j, narg_rgb = 0, narg_alpha = 0;
    GLint needtex[WES_MULTITEX_NUM];

    for(i = 0; i < WES_MULTITEX_NUM; i++){
        needtex[i] = 0;
    }

    //Add needed textures
    for(i = 0; i < WES_MULTITEX_NUM; i++){
        if (s->uTexture[i].Enable){
            if (s->uTexture[i].Mode != WES_FUNC_COMBINE){
                    needtex[i] = 1;
            } else {

                wes_frag_nargs(s, i, &narg_rgb, &narg_alpha);
                for(j = 0; j < narg_rgb; j++){
                    if (s->uTexture[i].Arg[j].RGBSrc == WES_SRC_TEXTURE){
                        needtex[i] = 1;
                    }
                    if (s->uTexture[i].Arg[j].RGBSrc == WES_SRC_TEXTURE0){
                        needtex[0] = 1;
                    }
                    if (s->uTexture[i].Arg[j].RGBSrc == WES_SRC_TEXTURE1){
                        needtex[1] = 1;
                    }
                    if (s->uTexture[i].Arg[j].RGBSrc == WES_SRC_TEXTURE2){
                        needtex[2] = 1;
                    }
                    if (s->uTexture[i].Arg[j].RGBSrc == WES_SRC_TEXTURE3){
                        needtex[3] = 1;
                    }
                }

                for(j = 0; j < narg_alpha; j++){
                    if (s->uTexture[i].Arg[j].AlphaSrc == WES_SRC_TEXTURE){
                        needtex[i] = 1;
                    }
                    if (s->uTexture[i].Arg[j].AlphaSrc == WES_SRC_TEXTURE0){
                        needtex[0] = 1;
                    }
                    if (s->uTexture[i].Arg[j].AlphaSrc == WES_SRC_TEXTURE1){
                        needtex[1] = 1;
                    }
                    if (s->uTexture[i].Arg[j].AlphaSrc == WES_SRC_TEXTURE2){
                        needtex[2] = 1;
                    }
                    if (s->uTexture[i].Arg[j].AlphaSrc == WES_SRC_TEXTURE3){
                        needtex[3] = 1;
                    }
                }
            }
        }
    }

    for(i = 0; i < WES_MULTITEX_NUM; i++){
        if (needtex[i])
            str += sprintf(str, "lowp vec4 tex%i = texture2D(uTexUnit[%i], vTexCoord[%i].xy);\n", i, i, i);
    }

    return (GLint)(str - buff);
}

GLint
wes_frag_tex(char* buff, progstate_t *s)
{
    char *str = buff;
    GLint i;

    str += sprintf(str, "lowp vec4 arg0, arg1, arg2;\n");
    str += wes_frag_tex_obtain(str, s);

    for(i = 0; i < WES_MULTITEX_NUM; i++)
    {
        if (s->uTexture[i].Enable){
            switch(s->uTexture[i].Mode)
            {
                case WES_FUNC_REPLACE:
                    str += sprintf(str, "gl_FragColor = tex%i;\n", i);
                    break;

                case WES_FUNC_MODULATE:
					str += sprintf(str, "gl_FragColor *= tex%i;\n", i);// *= tex%i;\n", i);
                    break;

                case WES_FUNC_DECAL:
                    str += sprintf(str, "gl_FragColor.rgb = mix(gl_FragColor.rgb, tex%i.rgb, tex%i.a);\n", i, i);
                    break;

                case WES_FUNC_BLEND:
                    str += sprintf(str, "gl_FragColor.rgb = mix(gl_FragColor.rgb, uTexEnvColor[%i].rgb, tex%i.rgb);\n", i, i);
                    str += sprintf(str, "gl_FragColor.a *= tex%i.a;\n", i);
                    break;

                case WES_FUNC_ADD:
                    str += sprintf(str, "gl_FragColor.rgb += tex%i.rgb;\n", i);
                    str += sprintf(str, "gl_FragColor.a *= tex%i.a;\n", i);
                    break;

                case WES_FUNC_COMBINE:
                    str += wes_frag_combine(str, s, i);
                    break;

                default:    PRINT_ERROR("ERROR: No valid MODE for TEX%i", i); break;
            }
        }
    }
    return (GLint)(str - buff);
}


GLvoid
wes_frag_build(char* buff, progstate_t *s)
{
    char *str = buff;
    int i;
    str += sprintf(str, "%s", frag_header);
    str += wes_frag_tex(str, s);
    str += wes_frag_fog(str, s);
    str += wes_frag_clipplane(str, s);
    str += wes_frag_alphatest(str, s);
    str += sprintf(str, "}\n");

#ifdef WES_PRINT_SHADER
    fprintf(stdout, "===== Fragment Shader =====\n");
    for(i = 0; i < WES_MULTITEX_NUM; i++){
        if (s->uTexture[i].Enable){
            fprintf(stdout, "TEX%i MODE %i \n", i, s->uTexture[i].Mode);
            fprintf(stdout, "TEX%i RGB COMB %i ALPHA COMB %i \n", i, s->uTexture[i].RGBCombine, s->uTexture[i].AlphaCombine);
            fprintf(stdout, "TEX%i RGB SRC = [%i, %i, %i] \n", i, s->uTexture[i].Arg[0].RGBSrc, s->uTexture[i].Arg[1].RGBSrc,s->uTexture[i].Arg[2].RGBSrc);
            fprintf(stdout, "TEX%i ALPHA SRC = [%i, %i, %i] \n", i, s->uTexture[i].Arg[0].AlphaSrc, s->uTexture[i].Arg[1].AlphaSrc,s->uTexture[i].Arg[2].AlphaSrc);
            fprintf(stdout, "TEX%i RGB OP = [%i, %i, %i] \n", i, s->uTexture[i].Arg[0].RGBOp, s->uTexture[i].Arg[1].RGBOp,s->uTexture[i].Arg[2].RGBOp);
            fprintf(stdout, "TEX%i ALPHA OP = [%i, %i, %i] \n", i, s->uTexture[i].Arg[0].AlphaOp, s->uTexture[i].Arg[1].AlphaOp,s->uTexture[i].Arg[2].AlphaOp);
        }
    }

    fprintf(stdout, "%s", buff);
    fprintf(stdout, "\n===========================\n");
    fflush(stdout);
#endif

}
