#include <vgui/IScheme.h>

namespace vgui2
{

const char *IScheme::GetFontName(const HFont& font)
{
	return NULL;
}

int ISchemeManager::GetProportionalScaledValueEx(HScheme scheme, int normalizedValue)
{
	return GetProportionalScaledValue(normalizedValue);
}

int ISchemeManager::GetProportionalNormalizedValueEx(HScheme scheme, int scaledValue)
{
	return GetProportionalNormalizedValue(scaledValue);;
}

}