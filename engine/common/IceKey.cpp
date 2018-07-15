#include "port.h"
#include "IceKey.hpp"

struct TIceSubKey
{
	DWORD dwValue[3];
};

static bool g_bIceSBoxesInitialised = false;
static DWORD g_pdwIceSBox[4][1024];

static const int g_piIceSMod[4][4] =
{
	{ 333, 313, 505, 369 },
{ 379, 375, 319, 391 },
{ 361, 445, 451, 397 },
{ 397, 425, 395, 505 }
};

static const int g_piIceSXor[4][4] =
{
	{ 0x83, 0x85, 0x9b, 0xcd },
{ 0xcc, 0xa7, 0xad, 0x41 },
{ 0x4b, 0x2e, 0xd4, 0x33 },
{ 0xea, 0xcb, 0x2e, 0x04 }
};

static const DWORD g_pdwIcePBox[32] =
{
	0x00000001, 0x00000080, 0x00000400, 0x00002000,
	0x00080000, 0x00200000, 0x01000000, 0x40000000,
	0x00000008, 0x00000020, 0x00000100, 0x00004000,
	0x00010000, 0x00800000, 0x04000000, 0x20000000,
	0x00000004, 0x00000010, 0x00000200, 0x00008000,
	0x00020000, 0x00400000, 0x08000000, 0x10000000,
	0x00000002, 0x00000040, 0x00000800, 0x00001000,
	0x00040000, 0x00100000, 0x02000000, 0x80000000
};

static const int g_piIceKeyRotation[16] =
{
	0, 1, 2, 3, 2, 1, 3, 0,
	1, 3, 2, 0, 3, 1, 0, 2
};

static DWORD GFMultiply(DWORD a, DWORD b, DWORD m)
{
	DWORD res = 0;

	while (b)
	{
		if (b & 1)
			res ^= a;

		a <<= 1;
		b >>= 1;

		if (a >= 256)
			a ^= m;
	}

	return res;
}

static DWORD GFExp7(DWORD b, DWORD m)
{
	DWORD x;

	if (b == 0)
		return 0;

	x = GFMultiply(b, b, m);
	x = GFMultiply(b, x, m);
	x = GFMultiply(x, x, m);
	return GFMultiply(b, x, m);
}

static DWORD IcePerm32(DWORD x)
{
	DWORD res = 0;
	const DWORD *pbox = g_pdwIcePBox;

	while (x)
	{
		if (x & 1)
			res |= *pbox;

		pbox++;
		x >>= 1;
	}

	return res;
}

static void IceSBoxesInitialize(void)
{
	int i;

	for (i = 0; i < 1024; ++i)
	{
		int col = (i >> 1) & 0xff;
		int row = (i & 0x1) | ((i & 0x200) >> 8);
		DWORD x;

		x = GFExp7(col ^ g_piIceSXor[0][row], g_piIceSMod[0][row]) << 24;
		g_pdwIceSBox[0][i] = IcePerm32(x);

		x = GFExp7(col ^ g_piIceSXor[1][row], g_piIceSMod[1][row]) << 16;
		g_pdwIceSBox[1][i] = IcePerm32(x);

		x = GFExp7(col ^ g_piIceSXor[2][row], g_piIceSMod[2][row]) << 8;
		g_pdwIceSBox[2][i] = IcePerm32(x);

		x = GFExp7(col ^ g_piIceSXor[3][row], g_piIceSMod[3][row]);
		g_pdwIceSBox[3][i] = IcePerm32(x);
	}
}

static DWORD IceFunction(DWORD dwValue, const TIceSubKey *sk)
{
	DWORD tl, tr;
	DWORD al, ar;

	tl = ((dwValue >> 16) & 0x3ff) | (((dwValue >> 14) | (dwValue << 18)) & 0xffc00);
	tr = (dwValue & 0x3ff) | ((dwValue << 2) & 0xffc00);

	al = sk->dwValue[2] & (tl ^ tr);
	ar = al ^ tr;
	al ^= tl;

	al ^= sk->dwValue[0];
	ar ^= sk->dwValue[1];

	return g_pdwIceSBox[0][al >> 10] | g_pdwIceSBox[1][al & 0x3ff] | g_pdwIceSBox[2][ar >> 10] | g_pdwIceSBox[3][ar & 0x3ff];
}

CIceKey::CIceKey(int n) : m_iSize(0), m_iRounds(0)
{
	if (!g_bIceSBoxesInitialised)
	{
		IceSBoxesInitialize();
		g_bIceSBoxesInitialised = 1;
	}

	if (n < 1)
	{
		m_iSize = 1;
		m_iRounds = 8;
	}
	else
	{
		m_iSize = n;
		m_iRounds = n * 16;
	}

	m_pKeySchedule = new TIceSubKey[m_iRounds];
}

CIceKey::~CIceKey(void)
{
	int i, j;

	for (i = 0; i < m_iRounds; ++i)
	{
		for (j = 0; j < 3; ++j)
			m_pKeySchedule[i].dwValue[j] = 0;
	}

	m_iRounds = m_iSize = 0;

	delete[] m_pKeySchedule;
}

void CIceKey::BuildSchedule(unsigned short *usKeyBuilder, int n, const int *cpiKeyRotation)
{
	int i, j, k;

	for (i = 0; i < 8; ++i)
	{
		int iKeyRotation = cpiKeyRotation[i];
		TIceSubKey *pSubKey = &m_pKeySchedule[n + i];

		for (j = 0; j < 3; ++j)
			pSubKey->dwValue[j] = 0;

		for (j = 0; j < 15; ++j)
		{
			DWORD *pdwCurrentSubKey = &pSubKey->dwValue[j % 3];

			for (k = 0; k < 4; ++k)
			{
				unsigned short *pusCurrentKeyBuilder = &usKeyBuilder[(iKeyRotation + k) & 3];
				unsigned short bit = *pusCurrentKeyBuilder & 1;

				*pdwCurrentSubKey = (*pdwCurrentSubKey << 1) | bit;
				*pusCurrentKeyBuilder = (*pusCurrentKeyBuilder >> 1) | ((bit ^ 1) << 15);
			}
		}
	}
}

void CIceKey::SetKey(const BYTE *pKey)
{
	int i, j;
	unsigned short pusKeyBuilder[4];

	if (m_iRounds == 8)
	{
		for (i = 0; i < 4; ++i)
			pusKeyBuilder[3 - i] = (pKey[i << 1] << 8) | pKey[(i << 1) + 1];

		this->BuildSchedule(pusKeyBuilder, 0, g_piIceKeyRotation);
		return;
	}

	for (i = 0; i < m_iSize; ++i)
	{
		for (j = 0; j < 4; ++j)
			pusKeyBuilder[3 - j] = (pKey[(i << 3) + (j << 1)] << 8) | pKey[(i << 3) + (j << 1) + 1];

		this->BuildSchedule(pusKeyBuilder, i << 3, g_piIceKeyRotation);
		this->BuildSchedule(pusKeyBuilder, m_iRounds - 8 - (i << 3), &g_piIceKeyRotation[8]);
	}
}

void CIceKey::Encrypt(const BYTE pPlainText[8], BYTE pCipherText[8]) const
{
	int i;
	DWORD l, r;

	l = (((DWORD)pPlainText[0]) << 24) | (((DWORD)pPlainText[1]) << 16) | (((DWORD)pPlainText[2]) << 8) | pPlainText[3];
	r = (((DWORD)pPlainText[4]) << 24) | (((DWORD)pPlainText[5]) << 16) | (((DWORD)pPlainText[6]) << 8) | pPlainText[7];

	for (i = 0; i < m_iRounds; i += 2)
	{
		l ^= IceFunction(r, &m_pKeySchedule[i]);
		r ^= IceFunction(l, &m_pKeySchedule[i + 1]);
	}

	for (i = 0; i < 4; ++i)
	{
		pCipherText[3 - i] = (BYTE)(r & 0xff);
		pCipherText[7 - i] = (BYTE)(l & 0xff);

		r >>= 8;
		l >>= 8;
	}
}

void CIceKey::Decrypt(const BYTE pCipherText[8], BYTE pPlainText[8]) const
{
	int i;
	DWORD l, r;

	l = (((DWORD)pCipherText[0]) << 24) | (((DWORD)pCipherText[1]) << 16) | (((DWORD)pCipherText[2]) << 8) | pCipherText[3];
	r = (((DWORD)pCipherText[4]) << 24) | (((DWORD)pCipherText[5]) << 16) | (((DWORD)pCipherText[6]) << 8) | pCipherText[7];

	for (i = m_iRounds - 1; i > 0; i -= 2)
	{
		l ^= IceFunction(r, &m_pKeySchedule[i]);
		r ^= IceFunction(l, &m_pKeySchedule[i - 1]);
	}

	for (i = 0; i < 4; ++i)
	{
		pPlainText[3 - i] = (BYTE)(r & 0xff);
		pPlainText[7 - i] = (BYTE)(l & 0xff);

		r >>= 8;
		l >>= 8;
	}
}
