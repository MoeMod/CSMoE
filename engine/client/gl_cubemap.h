#pragma once

#include <memory>

typedef struct mstudiotex_s mstudiotexture_t;

namespace xe {
    void CubeMap_Init();
    void CubeMap_Shutdown();
    void CubeMap_LoadTexture();
    void CubeMap_Bind(const mstudiotexture_t *ptexture);
}