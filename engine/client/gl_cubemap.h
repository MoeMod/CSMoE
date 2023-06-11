#pragma once

#include <memory>

typedef struct mstudiotex_s mstudiotexture_t;

namespace xe {
    void CubeMap_Init();
    void CubeMap_Shutdown();
    void CubeMap_LoadTexture();
    bool CubeMap_Bind(const mstudiotexture_t *ptexture);
    float CubeMap_envBrightMul();
    float CubeMap_envBrightAdd();
}