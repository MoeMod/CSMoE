#pragma once

#include <vgui_controls/PHandle.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Progressbar.h>

class CGameLoading : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CGameLoading, Panel);

public:
	CGameLoading(Panel *parent, const char *name);
	~CGameLoading();
	virtual void PerformLayout(void);
	
	void SetStatusText(const char *szStatusText);
	bool SetProgressPoint(int progressPoint);
	void SetProgressRange(int min, int max);
	void SetProgressVisible(bool bSet);
	void Activate();

	virtual void PaintBackground(); // 重画高清♂无码的载入tga

private:
	vgui2::Label *m_pStatusText;
	vgui2::ProgressBar *m_pProgress;
	int m_iRangeMin, m_iRangeMax;
};