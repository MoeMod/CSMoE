
#pragma once

class CHudBase_ZB2 : public CHudBase
{
	friend class CHudZB2;
};

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

	int MsgFunc_ZB2Msg(const char *pszName, int iSize, void *pbuf);

protected:
	class impl_t;
	impl_t *pimpl;
};