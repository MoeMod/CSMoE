#ifndef PUBLIC_UNICODEFILEHELPERS_H
#define PUBLIC_UNICODEFILEHELPERS_H

#include <tier0/platform.h>

wchar_t* AdvanceOverWhitespace(wchar_t* Start );

wchar_t* ReadToEndOfLine(wchar_t* start );

wchar_t* ReadUnicodeToken(wchar_t* start, wchar_t* token, int tokenBufferSize, bool& quoted );

#endif //PUBLIC_UNICODEFILEHELPERS_H
