//====== Copyright ?1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include <vgui/IVGUI.h>
#include "Controls.h"

#include "AnimatingImagePanel.h"
#include "BitmapImagePanel.h"
#include "ExpandButton.h"
#include "TreeViewListControl.h"
#include "HTML.h"

using namespace vgui2;

USING_BUILD_FACTORY( Button );
USING_BUILD_FACTORY( EditablePanel );
USING_BUILD_FACTORY( ImagePanel );
USING_BUILD_FACTORY( Label );
USING_BUILD_FACTORY( Panel );
USING_BUILD_FACTORY( ToggleButton );
USING_BUILD_FACTORY( AnimatingImagePanel );
USING_BUILD_FACTORY( CBitmapImagePanel );
USING_BUILD_FACTORY( CheckButton );
USING_BUILD_FACTORY( ComboBox );
USING_BUILD_FACTORY( Divider );
USING_BUILD_FACTORY( ExpandButton );
USING_BUILD_FACTORY( GraphPanel );
//USING_BUILD_FACTORY_ALIAS( HTML, HTML_NoJavascript );
//USING_BUILD_FACTORY_ALIAS( HTML, HTML_Javascript );
USING_BUILD_FACTORY( ListPanel );
USING_BUILD_FACTORY( ListViewPanel );
USING_BUILD_FACTORY( Menu );
USING_BUILD_FACTORY( MenuBar );
USING_BUILD_FACTORY( MenuButton );
USING_BUILD_FACTORY( MenuItem );
USING_BUILD_FACTORY( MessageBox );
USING_BUILD_FACTORY( ProgressBar );
USING_BUILD_FACTORY( RadioButton );
USING_BUILD_FACTORY( RichText );
USING_BUILD_FACTORY_ALIAS( ScrollBar, ScrollBar_Vertical );
USING_BUILD_FACTORY_ALIAS( ScrollBar, ScrollBar_Horizontal );
USING_BUILD_FACTORY( ScrollBar );
USING_BUILD_FACTORY( TextEntry );
USING_BUILD_FACTORY( TreeView );
USING_BUILD_FACTORY( CTreeViewListControl );
USING_BUILD_FACTORY( URLLabel );

int g_nYou_Must_Add_Public_Vgui_Controls_Vgui_ControlsCpp_To_Your_Project = 0;

#ifdef XASH_STATIC_GAMELIB

#define REGISTER_BUILD_FACTORY( className ) \
    extern vgui2::Panel *Create_##className( void ); \
    new vgui2::CBuildFactoryHelper( #className, Create_##className );

#define REGISTER_BUILD_FACTORY_DEFAULT_TEXT( className, defaultText ) \
    REGISTER_BUILD_FACTORY( className )

#define REGISTER_BUILD_FACTORY_CUSTOM( className, createFunc ) \
    extern vgui2::Panel *createFunc( void ); \
    new vgui2::CBuildFactoryHelper( #className, createFunc );

#define REGISTER_BUILD_FACTORY_CUSTOM_ALIAS( className, factoryName, createFunc ) \
    extern vgui2::Panel *createFunc( void ); \
    new vgui2::CBuildFactoryHelper( #factoryName, createFunc );

void RegisterControls()
{

REGISTER_BUILD_FACTORY_CUSTOM(CCvarToggleCheckButton, CvarToggleCheckButton_Factory);
REGISTER_BUILD_FACTORY(CFooterPanel);
REGISTER_BUILD_FACTORY(CCvarSlider);
//REGISTER_BUILD_FACTORY_DEFAULT_TEXT(URLButton, URLButton);
REGISTER_BUILD_FACTORY( AnalogBar );
REGISTER_BUILD_FACTORY( ContinuousAnalogBar );
REGISTER_BUILD_FACTORY( AnimatingImagePanel );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( CBitmapImagePanel, BitmapImagePanel );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( Button, Button );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( CheckButton, CheckButton );
REGISTER_BUILD_FACTORY( CircularProgressBar );
REGISTER_BUILD_FACTORY_CUSTOM( ComboBox, ComboBox_Factory );
REGISTER_BUILD_FACTORY( CControllerMap );
REGISTER_BUILD_FACTORY( Divider );
REGISTER_BUILD_FACTORY( EditablePanel );
REGISTER_BUILD_FACTORY( ExpandButton );
REGISTER_BUILD_FACTORY( GraphPanel );
REGISTER_BUILD_FACTORY(ImagePanel);
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( Label, Label );
REGISTER_BUILD_FACTORY( ListPanel );
REGISTER_BUILD_FACTORY( ListViewPanel );
REGISTER_BUILD_FACTORY(Menu);
REGISTER_BUILD_FACTORY( MenuBar );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( MenuButton, MenuButton );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( MenuItem, MenuItem );
REGISTER_BUILD_FACTORY_CUSTOM( MessageBox, MessageBox_Factory );
REGISTER_BUILD_FACTORY( Panel );
REGISTER_BUILD_FACTORY( ProgressBar );
REGISTER_BUILD_FACTORY( ContinuousProgressBar );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( RadioButton, RadioButton );
REGISTER_BUILD_FACTORY( RichText );
REGISTER_BUILD_FACTORY( RotatingProgressBar );
REGISTER_BUILD_FACTORY_CUSTOM_ALIAS(ScrollBar, ScrollBar_Vertical, ScrollBar_Vertical_Factory);
REGISTER_BUILD_FACTORY_CUSTOM_ALIAS(ScrollBar, ScrollBar_Horizontal, ScrollBar_Horizontal_Factory);
REGISTER_BUILD_FACTORY_CUSTOM(ScrollBar, ScrollBar_Horizontal_Factory);
REGISTER_BUILD_FACTORY( TextEntry );
REGISTER_BUILD_FACTORY_DEFAULT_TEXT( ToggleButton, ToggleButton );
REGISTER_BUILD_FACTORY( TreeView );
REGISTER_BUILD_FACTORY( CTreeViewListControl );
REGISTER_BUILD_FACTORY_CUSTOM( URLLabel, URLLabel_Factory );
    
}

#endif