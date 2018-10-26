
#pragma once

class CHudZB2 : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Reset(void) override;
	void InitHUDData(void) override;		// called every time a server is connected to
	void Shutdown(void) override;

	CHudMsgFunc(ZB2Msg);

protected:
	class impl_t;
	impl_t *pimpl;
};