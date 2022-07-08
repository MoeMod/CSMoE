#include <vgui/IInput.h>


// CS vgui2.dll doesn't have the following functions, so just return a default value
namespace vgui2
{

	void IInput::OnKeyCodeUnhandled(int keyCode)
	{
		return;
	}

	VPANEL IInput::GetModalSubTree(void)
	{
		return NULL;
	}

	bool IInput::ShouldModalSubTreeReceiveMessages(void) const
	{
		return false;
	}

	int IInput::GetIMELanguageList(LanguageItem *dest, int destcount)
	{
		return 0;
	}

	int IInput::GetIMEConversionModes(ConversionModeItem *dest, int destcount)
	{
		return 0;
	}

	int IInput::GetIMESentenceModes(SentenceModeItem *dest, int destcount)
	{
		return 0;
	}

	void IInput::OnChangeIME(bool forward)
	{
		return;
	}

	int IInput::GetCurrentIMEHandle(void)
	{
		return 0;
	}

	int IInput::GetEnglishIMEHandle(void)
	{
		return 0;
	}

	void IInput::GetIMELanguageShortCode(wchar_t *buf, int unicodeBufferSizeInBytes)
	{
		buf[0] = 0;
		return;
	}

	void IInput::SetCandidateWindowPos(int x, int y)
	{
		return;
	}

	bool IInput::GetShouldInvertCompositionString(void)
	{
		return false;
	}

	void IInput::OnChangeIMEByHandle(int handleValue)
	{
		return;
	}

	void IInput::OnChangeIMEConversionModeByHandle(int handleValue)
	{
		return;
	}

	void IInput::OnChangeIMESentenceModeByHandle(int handleValue)
	{
		return;
	}

	int IInput::GetCandidateListCount(void)
	{
		return 0;
	}

	void IInput::GetCandidate(int num, wchar_t *dest, int destSizeBytes)
	{
		dest[0] = 0;
		return;
	}

	int IInput::GetCandidateListSelectedItem(void)
	{
		return 0;
	}

	int IInput::GetCandidateListPageSize(void)
	{
		return 0;
	}

	int IInput::GetCandidateListPageStart(void)
	{
		return 0;
	}

	bool IInput::CandidateListStartsAtOne(void)
	{
		return 0;
	}

	void IInput::SetCandidateListPageStart(int start)
	{
		return;
	}

	VPANEL IInput::GetMouseCapture(void)
	{
		return NULL;
	}

}