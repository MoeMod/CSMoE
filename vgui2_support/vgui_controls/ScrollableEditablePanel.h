//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef SCROLLABLEEDITABLEPANEL_H
#define SCROLLABLEEDITABLEPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include "EditablePanel.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
namespace vgui2
{
	class ScrollBar;
}

namespace vgui2
{

//-----------------------------------------------------------------------------
// An editable panel that has a scrollbar
//-----------------------------------------------------------------------------
class ScrollableEditablePanel : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE( ScrollableEditablePanel, EditablePanel );

public:
	ScrollableEditablePanel( vgui2::Panel *pParent, vgui2::EditablePanel *pChild, const char *pName );
	virtual ~ScrollableEditablePanel() {}
	virtual void PerformLayout();

	MESSAGE_FUNC( OnScrollBarSliderMoved, "ScrollBarSliderMoved" );

private:
	vgui2::ScrollBar *m_pScrollBar;
	vgui2::EditablePanel *m_pChild;
};


} // end namespace vgui

#endif // SCROLLABLEEDITABLEPANEL_H