#pragma once
#include "tier1/strtools.h"

inline bool Q_StripUnprintableAndSpace(char *pch)
{
    return Q_RemoveAllEvilCharacters(pch);
}