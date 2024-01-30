#pragma once

typedef enum
{
    SSV_NORMAL,
    SSV_CUBEMAP,
    MAX_STUDIOSHADERVARIANTS
} studioshadervariant_e;

typedef struct studioshader_s {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    struct {
        GLint texOffset;
        GLint colorMix;
        GLint lightInfo;
        GLint dLightDir;
        GLint worldEye;
        GLint fogInfo;
        GLint param;
        GLint outline;
        GLint texDiffuseMap;
        GLint texEnvMap;
        GLint glowColor;
        GLint fogColor;
    } uniform;
    struct {
        GLint aPos;
        GLint aNormal;
        GLint aTexCoord;
    } attrib;
} studioshader_t;

typedef struct studioubo_s {
	struct Matrices
	{
		matrix4x4 ModelView;
		matrix4x4 Projection;
		matrix3x4 BoneMat[MAXSTUDIOBONES];
	};

	GLuint Matrices_ubo;
	static const int MatricesBindingPoint = 0;;
} studioubo_t;

extern studioshader_t g_studioshadervariant[MAX_STUDIOSHADERVARIANTS];
extern studioshader_t *g_pstudioshader;