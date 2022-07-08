
#include "CBackGroundPanel.h"
#include <vgui/ISurface.h>

CBackGroundPanel::CBackGroundPanel( vgui2::Panel* pParent )
	: BaseClass( pParent, "ViewPortBackGround" )
{
	SetScheme( "ClientScheme" );

	SetTitleBarVisible( false );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( true );
}

void CBackGroundPanel::ApplySchemeSettings( vgui2::IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	const auto color = pScheme->GetColor( "ViewportBG", Color( 0, 0, 0, 0 ) );

	SetBgColor( color );
}

void CBackGroundPanel::PerformLayout()
{
	//Resize ourselves to the screen's size to fill the entire viewport.
	int w, h;

	vgui2::surface()->GetScreenSize( w, h );

	SetBounds( 0, 0, w, h );

	BaseClass::PerformLayout();
}
