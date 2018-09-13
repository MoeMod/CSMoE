
#pragma once

class CHudBase_ZBS : protected CHudBase
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

protected:
	class impl_t;
	impl_t *pimpl;
};