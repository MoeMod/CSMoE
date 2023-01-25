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
        GLint boneMat;
        GLint muWVP;
        GLint worldEye;
        GLint texDiffuseMap;
        GLint texEnvMap;
        GLint glowColor;
    } uniform;
    struct {
        GLint vaPosition;
        GLint vaNormal;
        GLint vaTexCoord;
    } attrib;
} studioshader_t;
extern studioshader_t g_studioshadervariant[MAX_STUDIOSHADERVARIANTS];
extern studioshader_t *g_pstudioshader;