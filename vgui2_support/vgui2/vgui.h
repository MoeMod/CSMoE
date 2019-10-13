#ifndef VGUI2_SRC_VGUI_H
#define VGUI2_SRC_VGUI_H

#include <tier1/UtlLinkedList.h>
#include <tier1/UtlPriorityQueue.h>

#include <vgui/VGUI2.h>
#include <vgui/IInputInternal.h>
#include <vgui/IVGui.h>

namespace vgui2
{
class VPanel;

struct SerialPanel_t
{
	unsigned short m_iSerialNumber;
	VPANEL m_pPanel;
};
}

struct MessageItem_t
{
	KeyValues *_params;
	vgui2::HPanel _messageTo;
	vgui2::HPanel _from;
	float _arrivalTime;
	int _messageID;
};

class CVGui : public vgui2::IVGui
{
private:
	struct Tick_t
	{
		vgui2::VPANEL panel;
		int interval;
		int nexttick;
		char panelname[ 64 ];
	};

	struct Context_t
	{
		vgui2::HInputContext m_hInputContext;
	};

public:
	CVGui();
	~CVGui();

	//IVGui

	bool Init( CreateInterfaceFn *factoryList, int numFactories ) override;

	void Shutdown() override;

	void Start() override;

	void Stop() override;

	bool IsRunning() override;

	void RunFrame() override;

	void ShutdownMessage( unsigned int shutdownID ) override;

	vgui2::VPANEL AllocPanel() override;
	void FreePanel( vgui2::VPANEL panel ) override;

	void DPrintf( const char *format, ... ) override;
	void DPrintf2( const char *format, ... ) override;
	void SpewAllActivePanelNames() override;

	vgui2::HPanel PanelToHandle( vgui2::VPANEL panel ) override;
	vgui2::VPANEL HandleToPanel( vgui2::HPanel index ) override;
	void MarkPanelForDeletion( vgui2::VPANEL panel ) override;

	void AddTickSignal( vgui2::VPANEL panel, int intervalMilliseconds = 0 ) override;
	void RemoveTickSignal( vgui2::VPANEL panel ) override;

	void PostMessage( vgui2::VPANEL target, KeyValues *params, vgui2::VPANEL from, float delaySeconds = 0.0f ) override;

	vgui2::HContext CreateContext() override;
	void DestroyContext( vgui2::HContext context ) override;

	void AssociatePanelWithContext( vgui2::HContext context, vgui2::VPANEL pRoot ) override;

	void ActivateContext( vgui2::HContext context ) override;

	void SetSleep( bool state ) override;

	bool GetShouldVGuiControlSleep() override;

private:
	void PanelCreated( vgui2::VPanel* panel );
	void PanelDeleted( vgui2::VPanel* panel );

	void ClearMessageQueues();

	bool DispatchMessages();

private:
	bool m_bRunning;

	CUtlLinkedList<vgui2::SerialPanel_t, unsigned short> m_PanelList;

	unsigned short m_iNextSerialNumber = 0;

	bool m_bInitialized = false;

	int m_iCurrentMessageID;

	bool m_bDoSleep = true;

	CUtlVector<CVGui::Tick_t*> m_TickSignalVec;

	CUtlLinkedList<CVGui::Context_t, unsigned short> m_Contexts;

	bool m_InDispatcher;

	vgui2::HContext m_hContext = vgui2::DEFAULT_VGUI_CONTEXT;
	CVGui::Context_t m_DefaultContext = { vgui2::DEFAULT_INPUT_CONTEXT };

	CUtlLinkedList<MessageItem_t, unsigned short> m_MessageQueue;
	CUtlLinkedList<MessageItem_t, unsigned short> m_SecondaryQueue;
	CUtlPriorityQueue<MessageItem_t> m_DelayedMessageQueue;

private:
	CVGui( const CVGui& ) = delete;
	CVGui& operator=( const CVGui& ) = delete;
};

namespace vgui2
{
void vgui_strcpy( char* dst, int dstLen, const char* src );
}

#endif //VGUI2_SRC_VGUI_H
