
#include <interface.h>
#include "vgui/VGUI.h"
#include "vgui/ISchemeManager.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/controls.h"
#include "xash3d_types.h"

#include <string>
#include <string_view>

static vgui2::HFont _consoleFont = NULL_HANDLE;
extern rgba_t g_color_table[8];

void VGUI2_Draw_Init()
{
    auto scheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetDefaultScheme());
    _consoleFont = scheme->GetFont("Default");
}

int VGUI2_GetFontWide( int ch, unsigned int font )
{
    int a, b, c;
    vgui2::surface()->GetCharABCwide( font, ch, a, b, c );

    return c + b + a;
}

int VGUI2_GetFontTall( unsigned int font )
{
    return vgui2::surface()->GetFontTall( font );
}

std::basic_string<uchar32> VGUI2_Find_String( const char* str )
{
    auto size_in_bytes = Q_UTF8ToUTF32(str, nullptr, 0);
    std::basic_string<uchar32> out(size_in_bytes / sizeof(uchar32), '\0');
    Q_UTF8ToUTF32(str, out.data(), size_in_bytes);
    return out;
}

int VGUI2_Draw_StringLenW( std::basic_string_view<uchar32> wsv, unsigned int font )
{
    int len = 0;

    for( size_t i = 0; i < wsv.length(); ++i )
    {
        len += VGUI2_GetFontWide( wsv[ i ], font );
    }

    return len;
}

int VGUI2_Surface_GetCharWidth(int ch)
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

int VGUI2_Surface_DrawChar(int x, int y, int ch, byte r, byte g, byte b, byte a)
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

int VGUI2_Surface_DrawConsoleString(int x0, int y0, const char* string, byte r, byte g, byte b, byte a)
{
    //g_BaseUISurface._engineSurface->resetViewPort();
    auto font = _consoleFont;

    vgui2::surface()->DrawSetTextFont( font );

    auto pszString = VGUI2_Find_String( string );

    static auto print_segment = [](int x, int y, std::basic_string_view<uchar32> sv, byte r, byte g, byte b, byte a)
    {
        vgui2::surface()->DrawSetTextColor( r * 0.55, g * 0.34, b * 0.11, a );
        vgui2::surface()->DrawSetTextPos( x - 1, y - 1 );
        vgui2::surface()->DrawPrintText( sv.data(), sv.length() );
        vgui2::surface()->DrawSetTextPos( x - 1, y + 1 );
        vgui2::surface()->DrawPrintText( sv.data(), sv.length() );
        vgui2::surface()->DrawSetTextPos( x + 1, y - 1 );
        vgui2::surface()->DrawPrintText( sv.data(), sv.length() );
        vgui2::surface()->DrawSetTextPos( x + 1, y + 1 );
        vgui2::surface()->DrawPrintText( sv.data(), sv.length() );


        vgui2::surface()->DrawSetTextColor( 0, 0, 0, a );
        vgui2::surface()->DrawSetTextPos( x + 2, y + 2 );
        vgui2::surface()->DrawPrintText( sv.data(), sv.length() );

        vgui2::surface()->DrawSetTextColor( r, g, b, a );
        vgui2::surface()->DrawSetTextPos( x, y );
        vgui2::surface()->DrawPrintText( sv.data(), sv.length() );
    };

    static auto print_char_no_shadow = [](int x, int y, uchar32 ch, byte r, byte g, byte b, byte a)
    {
        vgui2::surface()->DrawSetTextColor( r, g, b, a );
        vgui2::surface()->DrawSetTextPos( x, y );
        vgui2::surface()->DrawUnicodeChar( ch );
    };

    std::basic_string_view<uchar32> sv = pszString;
    rgba_t col = {r, g, b, a};
    std::decay<rgba_t>::type last_color = col;
    int x = x0, y = y0;
    auto find_unary = [](uchar32 ch) { return (ch >= U'\x01' && ch <= U'\x07') || (ch == U'\n') || (ch == U'^') || vgui2::surface()->IsEmojiChar(ch); };
    for (auto iter = std::find_if(sv.begin(), sv.end(), find_unary); iter != sv.end(); iter = std::find_if(sv.begin(), sv.end(), find_unary))
    {
        auto seg = iter - sv.begin();
        if (sv[seg] == U'^' && seg != sv.size() - 1 && sv[seg + 1] >= U'1' && sv[seg + 1] <= U'7')
        {
            print_segment(x, y, sv.substr(0, seg), last_color[0], last_color[1], last_color[2], last_color[3]);
            auto size = VGUI2_Draw_StringLenW(sv.substr(0, seg), font);
            x += size;

            if (sv[seg + 1] == U'7')
                last_color = col;
            else
                last_color = g_color_table[sv[seg + 1] - U'0'];

            sv = sv.substr(seg + 2);
            continue;
        }
        else if (sv[seg] >= U'\x01' && sv[seg] <= U'\x07' && seg != sv.size() - 1)
        {
            print_segment(x, y, sv.substr(0, seg), last_color[0], last_color[1], last_color[2], last_color[3]);
            auto size = VGUI2_Draw_StringLenW(sv.substr(0, seg), font);
            x += size;
            sv = sv.substr(seg + 1);
            continue;
        }
        else if (vgui2::surface()->IsEmojiChar(sv[seg]))
        {
            print_segment(x, y, sv.substr(0, seg), last_color[0], last_color[1], last_color[2], last_color[3]);
            auto size = VGUI2_Draw_StringLenW(sv.substr(0, seg), font);
            x += size;

            print_char_no_shadow(x, y, sv[seg], last_color[0], last_color[1], last_color[2], last_color[3]);
            size = VGUI2_GetFontWide( sv[seg], font );
            x += size;

            sv = sv.substr(seg + 1);
            continue;
        }
        else if (sv[seg] == '\n' && seg != sv.size() - 1)
        {
            print_segment(x, y, sv.substr(0, seg), last_color[0], last_color[1], last_color[2], last_color[3]);
            auto size = VGUI2_Draw_StringLenW(sv.substr(0, seg), font);
            y += VGUI2_GetFontTall(font);
            x = x0;
            sv = sv.substr(seg + 1);
            continue;
        }
        break;
    }
    if (!sv.empty())
    {
        print_segment(x, y, sv, last_color[0], last_color[1], last_color[2], last_color[3]);
        auto size = VGUI2_Draw_StringLenW(sv, font);
        x += size;
    }

    vgui2::surface()->DrawFlushText();
    return x - x0;
}