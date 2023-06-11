/*
gl_studiolru.h
Copyright (C) 2023 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "gl_export.h"

typedef struct studio_vertex_attrib_item_s {
    GLfloat pos[3];
    GLfloat boneIdx;
    GLfloat coord[2];
    GLubyte norm[3];
} studio_vertex_attrib_item_t;

namespace xe {
    void StudioLru_Init();
    void StudioLru_Shutdown();
    // return true if already has result, false if new vbo is created
    void StudioLru_BindVBO(const studiohdr_t *studiohdr, const mstudiomodel_t *submodel, int mesh, int flags);
    void StudioLru_UnbindVBO();
    bool StudioLru_HasCachedData();
    void StudioLru_Submit(int startArrayElems, int numArrayElems, int startArrayVerts, int numArrayVerts);
    void StudioLru_DrawMesh();
    void StudioLru_Shrink();
}