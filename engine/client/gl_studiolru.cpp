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

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "studio.h"


#include "gl_studioshader.h"
#include "gl_studiolru.h"

#include "util/lru_cache.h"

namespace xe {
    using studiolru_key_t = std::tuple<const studiohdr_t *, const mstudiomodel_t *, int, int>; // model, submodel, mesh, flags
    class studiolru_vbo_cache
    {
    public:
        studiolru_vbo_cache(const char *debugname);
        ~studiolru_vbo_cache();

        void Bind();
        void Unbind();
        bool HasCachedData() const;
        void Submit(int startArrayElems, int numArrayElems);
        void DrawElements() const;

    private:
        std::string name;
        GLuint studioverts;
        GLuint studioelems;
        int start;
        int num;
    };

    studiolru_vbo_cache::studiolru_vbo_cache(const char *debugname)
    {
        name = debugname;
        pglGenBuffersARB( 1, &studioverts );
        pglGenBuffersARB( 1, &studioelems);
        start = 0;
        num = 0;

        pglBindBufferARB( GL_ARRAY_BUFFER_ARB, studioverts );
        pglBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, studioelems);
        pglBufferDataARB( GL_ARRAY_BUFFER_ARB, MAXARRAYVERTS * sizeof(studio_vertex_attrib_item_t), nullptr, GL_STATIC_DRAW_ARB);
        pglBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, MAXARRAYVERTS * 6 * sizeof(unsigned short), nullptr, GL_STATIC_DRAW_ARB);
        pglBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
        pglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        MsgDev(D_INFO, "StudioLru : loading model vbo %s\n", name.c_str());
    }

    studiolru_vbo_cache::~studiolru_vbo_cache() {
        pglDeleteBuffersARB( 1, &studioverts );
        pglDeleteBuffersARB( 1, &studioelems);
        MsgDev(D_INFO, "StudioLru : swap out model vbo %s\n", name.c_str());
    }

    void studiolru_vbo_cache::Bind()
    {
        pglBindBufferARB( GL_ARRAY_BUFFER_ARB, studioverts );
        pglBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, studioelems );
    }

    void studiolru_vbo_cache::Unbind()
    {
        pglBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
        pglBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }

    bool studiolru_vbo_cache::HasCachedData() const
    {
        return num > 0;
    }

    void studiolru_vbo_cache::Submit(int startArrayElems, int numArrayElems)
    {
        start = startArrayElems;
        num = numArrayElems;
    }

    void studiolru_vbo_cache::DrawElements() const {
        pglEnableVertexAttribArrayARB(g_pstudioshader->attrib.vaPosition);
        pglEnableVertexAttribArrayARB(g_pstudioshader->attrib.vaTexCoord);
        pglEnableVertexAttribArrayARB(g_pstudioshader->attrib.vaNormal);
        pglVertexAttribPointerARB(g_pstudioshader->attrib.vaPosition, 3 + 1, GL_FLOAT, GL_FALSE, sizeof(studio_vertex_attrib_item_t), (const GLvoid *)offsetof(studio_vertex_attrib_item_s, pos));
        pglVertexAttribPointerARB(g_pstudioshader->attrib.vaTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(studio_vertex_attrib_item_t), (const GLvoid *)offsetof(studio_vertex_attrib_item_s, coord));
        pglVertexAttribPointerARB(g_pstudioshader->attrib.vaNormal, 3, GL_FLOAT, GL_FALSE, sizeof(studio_vertex_attrib_item_t), (const GLvoid *)offsetof(studio_vertex_attrib_item_s, norm));
        pglDrawElements( GL_TRIANGLES, num, GL_UNSIGNED_SHORT, (const GLvoid *)(start * sizeof(unsigned short)) );
        pglDisableVertexAttribArrayARB(g_pstudioshader->attrib.vaPosition);
        pglDisableVertexAttribArrayARB(g_pstudioshader->attrib.vaTexCoord);
        pglDisableVertexAttribArrayARB(g_pstudioshader->attrib.vaNormal);
    }

    static lru_cache<studiolru_key_t, std::shared_ptr<studiolru_vbo_cache>> studiovbocache(512);
    static std::shared_ptr<studiolru_vbo_cache> current_studiolru_vbo;

    bool StudioLru_HasCachedData()
    {
        return current_studiolru_vbo->HasCachedData();
    }

    void StudioLru_BindVBO(const studiohdr_t *studiohdr, const mstudiomodel_t *submodel, int mesh, int flags)
    {
        studiolru_key_t key{ studiohdr, submodel, mesh, flags };
        current_studiolru_vbo = studiovbocache.get(key).value_or(nullptr);
        if (current_studiolru_vbo == nullptr)
        {
            auto new_vbo = std::make_shared<studiolru_vbo_cache>(studiohdr->name);
            studiovbocache.insert(key, new_vbo);
            current_studiolru_vbo = new_vbo;
        }
        current_studiolru_vbo->Bind();
    }

    void StudioLru_UnbindVBO()
    {
        current_studiolru_vbo->Unbind();
        current_studiolru_vbo = nullptr;
    }

    void StudioLru_Submit(int startArrayElems, int numArrayElems)
    {
        current_studiolru_vbo->Submit(startArrayElems, numArrayElems);
    }

    void StudioLru_DrawMesh()
    {
        current_studiolru_vbo->DrawElements();
    }

    void StudioLru_Init()
    {
        current_studiolru_vbo = nullptr;
        studiovbocache.clear();
    }

    void StudioLru_Shutdown()
    {
        current_studiolru_vbo = nullptr;
        studiovbocache.clear();
    }

    void StudioLru_Shrink()
    {
        while( studiovbocache.size() >= 64 ){
            // cache is full, evict the least recently used item
            studiovbocache.evict();
        }
    }
}
