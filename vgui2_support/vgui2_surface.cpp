
#include <interface.h>
#include "vgui/VGUI.h"
#include "vgui/ISchemeManager.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/controls.h"

static vgui2::HFont _consoleFont = NULL_HANDLE;

void VGUI2_Draw_Init()
{
    auto scheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetDefaultScheme());
    _consoleFont = scheme->GetFont("Default");
}

int VGUI2_GetFontWide( wchar_t ch, unsigned int font )
{
    int a, b, c;
    vgui2::surface()->GetCharABCwide( font, ch, a, b, c );

    return c + b + a;
}

int VGUI2_GetFontTall( unsigned int font )
{
    return vgui2::surface()->GetFontTall( font );
}

const wchar_t* VGUI2_Find_String( const char* str )
{
    static wchar_t tmpString[ 1024 ];
    /*
    if( str && *str == '#' )
    {
        auto pszLocalized = vgui2::localize()->Find( str );

        return pszLocalized;
    }
    */
    vgui2::localize()->ConvertANSIToUnicode( str, tmpString, sizeof( tmpString ) );

    return tmpString;
}

int VGUI2_Draw_StringLenW( const wchar_t* wsz, unsigned int font )
{
    int len = 0;

    for( size_t i = 0; i < wcslen( wsz ); ++i )
    {
        len += VGUI2_GetFontWide( wsz[ i ], font );
    }

    return len;
}

int VGUI2_Surface_GetCharWidth(wchar_t ch)
{
    return VGUI2_GetFontWide(ch, _consoleFont);
}

int VGUI2_Surface_GetCharHeight()
{
    return VGUI2_GetFontTall(_consoleFont);
}

int VGUI2_Draw_StringLen( const char* psz, unsigned int font )
{
    return VGUI2_Draw_StringLenW( VGUI2_Find_String( psz ), font );
}

int VGUI2_Surface_DrawChar(int x, int y, wchar_t ch, byte r, byte g, byte b, byte a)
{
    //g_BaseUISurface._engineSurface->resetViewPort();

    auto font = _consoleFont;

    vgui2::surface()->DrawSetTextFont( font );
    vgui2::surface()->DrawSetTextPos( x, y );
    vgui2::surface()->DrawSetTextColor( r, g, b, a );

    if( iswprint( ch ) )
    {
        vgui2::surface()->DrawUnicodeChar( ch );
        vgui2::surface()->DrawFlushText();
    }

    auto w = VGUI2_GetFontWide( ch, font );

    return w;
}

void VGUI2_Surface_DrawStringLen( const char* pText, int* length, int* height )
{
    if(length)
        *length = VGUI2_Draw_StringLenW( VGUI2_Find_String( pText ), _consoleFont );
    if(height)
        *height = VGUI2_GetFontTall(_consoleFont);
}

int VGUI2_Surface_DrawConsoleString(int x, int y, const char* string, byte r, byte g, byte b, byte a)
{
    //g_BaseUISurface._engineSurface->resetViewPort();
    auto font = _consoleFont;

    vgui2::surface()->DrawSetTextFont( font );
    vgui2::surface()->DrawSetTextPos( x, y );
    vgui2::surface()->DrawSetTextColor( r, g, b, a );

    auto pszString = VGUI2_Find_String( string );

    for( size_t i = 0; i < wcslen( pszString ); ++i )
    {
        if( !iswprint( pszString[ i ] ) )
        {
            //TODO: this could modify localized strings managed by ILocalize - Solokiller
            const_cast<wchar_t*>( pszString )[ i ] = L' ';
        }
    }

    vgui2::surface()->DrawPrintText( pszString, wcslen( pszString ) );
    vgui2::surface()->DrawFlushText();

    auto w = VGUI2_Draw_StringLenW( pszString, font );
    return w;
}