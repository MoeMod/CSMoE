#pragma once

#include "port.h"

struct TIceSubKey;

class CIceKey
{
public:
	CIceKey(int n = 0);
	~CIceKey(void);

	void SetKey(const BYTE *pKey);

	void Encrypt(const BYTE pPlainText[8], BYTE pCipherText[8]) const;
	void Decrypt(const BYTE pCipherText[8], BYTE pPlainText[8]) const;

	int GetKeySize() const { return m_iSize << 3; }
	static int GetBlockSize() { return 8; }

private:
	void BuildSchedule(unsigned short *usKeyBuilder, int n, const int *cpiKeyRotation);

	int m_iSize;
	int m_iRounds;
	TIceSubKey *m_pKeySchedule;
};
