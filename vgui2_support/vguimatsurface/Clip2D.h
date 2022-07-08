#ifndef CLIP2D_H
#define CLIP2D_H

namespace vgui2
{
	struct Vertex_t;
}

void EnableScissor(bool enable);
void SetScissorRect(int left, int top, int right, int bottom);
void GetScissorRect(int &left, int &top, int &right, int &bottom, bool &enabled);
bool ClipLine(const vgui2::Vertex_t *pInVerts, vgui2::Vertex_t *pOutVerts);
bool ClipRect(const vgui2::Vertex_t &inUL, const vgui2::Vertex_t &inLR, vgui2::Vertex_t *pOutUL, vgui2::Vertex_t *pOutLR);
int ClipPolygon(int iCount, vgui2::Vertex_t *pVerts, int iTranslateX, int iTranslateY, vgui2::Vertex_t ***pppOutVertex);

#endif