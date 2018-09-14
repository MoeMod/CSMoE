
#pragma once

class CHudBase_ZBS : public CHudBase
{
	friend class CHudZBS;
};

class CHudZBS : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Reset(void) override;
	void InitHUDData(void) override;		// called every time a server is connected to
	void Shutdown(void) override;

	int MsgFunc_ZBSKill(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_ZBSLevel(const char *pszName, int iSize, void *pbuf);

protected:
	class impl_t;
	impl_t *pimpl;
};