#include <vgui/ISurface.h>

namespace vgui2
{

int ISurface::DrawGetTextureId(char const *filename)
{
	return -1;
}

bool ISurface::DrawGetTextureFile(int id, char *filename, int maxlen)
{
	return false;
}

float __surface_alpha_multiplier = 0.0;

void ISurface::DrawSetAlphaMultiplier(float alpha)
{
	__surface_alpha_multiplier = alpha;
	return;
}

float ISurface::DrawGetAlphaMultiplier(void)
{
	return __surface_alpha_multiplier;
}

void ISurface::DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal)
{
	return;
}

const char *ISurface::GetResolutionKey(void) const
{
	return NULL;
}

void ISurface::DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1)
{
	return;
}

void ISurface::DrawTexturedPolygon(int n, Vertex_t *pVertices)
{
	return;
}

IImage *ISurface::GetIconImageForFullPath(char const *pFullPath)
{
	return NULL;
}

}