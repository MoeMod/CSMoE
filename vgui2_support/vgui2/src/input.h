#ifndef VGUI2_SRC_INPUT_H
#define VGUI2_SRC_INPUT_H

#include <tier1/UtlLinkedList.h>
#include <vgui/IInputInternal.h>

namespace vgui2
{
	class VPanel;
}

static const int MAX_MOUSE_BUTTONS = vgui2::MOUSE_LAST;
static const int MAX_KEYS = vgui2::KEY_SCROLLLOCKTOGGLE + 1;

typedef struct InputContext_s
{
	vgui2::VPANEL _rootPanel;

	bool _mousePressed[MAX_MOUSE_BUTTONS];
	bool _mouseDoublePressed[MAX_MOUSE_BUTTONS];
	bool _mouseDown[MAX_MOUSE_BUTTONS];
	bool _mouseReleased[MAX_MOUSE_BUTTONS];
	bool _keyPressed[MAX_KEYS];
	bool _keyTyped[MAX_KEYS];
	bool _keyDown[MAX_KEYS];
	bool _keyReleased[MAX_KEYS];

	vgui2::VPanel* _keyFocus;
	vgui2::VPanel* _oldMouseFocus;
	vgui2::VPanel* _mouseFocus;
	vgui2::VPanel* _mouseOver;
	vgui2::VPanel* _mouseCapture;
	vgui2::VPanel* _appModalPanel;

	int m_nCursorX;
	int m_nCursorY;
} InputContext_t;

class CInputWin32 : public vgui2::IInputInternal
{

public:
	CInputWin32();
	~CInputWin32();

	void SetMouseFocus(vgui2::VPANEL newMouseFocus) override;
	void SetMouseCapture(vgui2::VPANEL panel) override;

	void GetKeyCodeText(vgui2::KeyCode code, char *buf, int buflen) override;

	vgui2::VPANEL GetFocus() override;
	vgui2::VPANEL GetMouseOver() override;

	void SetCursorPos(int x, int y) override;
	void GetCursorPos(int &x, int &y) override;
	bool WasMousePressed(vgui2::MouseCode code) override;
	bool WasMouseDoublePressed(vgui2::MouseCode code) override;
	bool IsMouseDown(vgui2::MouseCode code) override;

	void SetCursorOveride(vgui2::HCursor cursor) override;
	vgui2::HCursor GetCursorOveride() override;

	bool WasMouseReleased(vgui2::MouseCode code) override;
	bool WasKeyPressed(vgui2::KeyCode code) override;
	bool IsKeyDown(vgui2::KeyCode code) override;
	bool WasKeyTyped(vgui2::KeyCode code) override;
	bool WasKeyReleased(vgui2::KeyCode code) override;

	vgui2::VPANEL GetAppModalSurface() override;

	void SetAppModalSurface(vgui2::VPANEL panel) override;

	void ReleaseAppModalSurface() override;

	void GetCursorPosition(int &x, int &y) override;

	void RunFrame() override;

	void UpdateMouseFocus(int x, int y) override;

	void PanelDeleted(vgui2::VPANEL panel) override;

	void InternalCursorMoved(int x, int y) override;
	void InternalMousePressed(vgui2::MouseCode code) override;
	void InternalMouseDoublePressed(vgui2::MouseCode code) override;
	void InternalMouseReleased(vgui2::MouseCode code) override;
	void InternalMouseWheeled(int delta) override;
	void InternalKeyCodePressed(vgui2::KeyCode code) override;
	void InternalKeyCodeTyped(vgui2::KeyCode code) override;
	void InternalKeyTyped(wchar_t unichar) override;
	void InternalKeyCodeReleased(vgui2::KeyCode code) override;

	vgui2::HInputContext CreateInputContext() override;
	void DestroyInputContext(vgui2::HInputContext context) override;

	void AssociatePanelWithInputContext(vgui2::HInputContext context, vgui2::VPANEL pRoot) override;

	void ActivateInputContext(vgui2::HInputContext context) override;

	vgui2::VPANEL GetMouseCapture() override;

	bool IsChildOfModalPanel(vgui2::VPANEL panel) override;

	void ResetInputContext(vgui2::HInputContext context) override;

public:
	void InitInputContext(InputContext_t* pContext);

	InputContext_t* GetContext(vgui2::HInputContext context);

	InputContext_t* GetCurrentContext();

	void PanelDeleted(vgui2::VPanel* pFocus, InputContext_t& context);

	vgui2::VPanel* CalculateNewKeyFocus();

public:
	bool _updateToggleButtonState = false;
	char* _keyTrans[MAX_KEYS];

	InputContext_t m_DefaultInputContext;
	vgui2::HInputContext m_hContext = vgui2::DEFAULT_INPUT_CONTEXT;
	CUtlLinkedList<InputContext_t, int> m_Contexts;

private:
	CInputWin32(const CInputWin32&) = delete;
	CInputWin32& operator=(const CInputWin32&) = delete;
};

#endif //VGUI2_SRC_INPUT_H
