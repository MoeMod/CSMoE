// Purpose: C++ implementation of the ICE encryption algorithm.
//			Taken from public domain code, as written by Matthew Kwan - July 1996
//			http://www.darkside.com.au/ice/

#if !defined(_STATIC_LINKED) || defined(_SHARED_LIB)

#include "mathlib/IceKey.H"
#include "tier0/memdbgon.h"

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM) || defined(_M_ARM64)
#include <arm_neon.h>
#define ICEKEY_NEON
#endif

#pragma warning(disable: 4244)


	/* Structure of a single round subkey */
class IceSubkey {
    public:
	uint32_t	val[3];
};


	/* The S-boxes */
#ifdef ICEKEY_NEON
static uint32x4_t	ice_sbox[1024];
#else
static uint32_t	ice_sbox[4][1024];
#endif
static bool		ice_sboxes_initialised = false;


	/* Modulo values for the S-boxes */
#if 0
static const uint32x4_t	ice_smod[4] = {
		{333, 379, 361, 397},
		{313, 375, 445, 425},
		{505, 319, 451, 395},
		{369, 391, 397, 505}};
#else
static const uint32_t	ice_smod[4][4] = {
				{333, 313, 505, 369},
				{379, 375, 319, 391},
				{361, 445, 451, 397},
				{397, 425, 395, 505}};
#endif

	/* XOR values for the S-boxes */
#if 0
static const uint32x4_t	ice_sxor[4] = {
		{0x83, 0xcc, 0x4b, 0xea},
		{0x85, 0xa7, 0x2e, 0xcb},
		{0x9b, 0xad, 0xd4, 0x2e},
		{0xcd, 0x41, 0x33, 0x04}};
#else
static const uint32_t	ice_sxor[4][4] = {
				{0x83, 0x85, 0x9b, 0xcd},
				{0xcc, 0xa7, 0xad, 0x41},
				{0x4b, 0x2e, 0xd4, 0x33},
				{0xea, 0xcb, 0x2e, 0x04}};
#endif

	/* Permutation values for the P-box */
static const uint32_t	ice_pbox[32] = {
		0x00000001, 0x00000080, 0x00000400, 0x00002000,
		0x00080000, 0x00200000, 0x01000000, 0x40000000,
		0x00000008, 0x00000020, 0x00000100, 0x00004000,
		0x00010000, 0x00800000, 0x04000000, 0x20000000,
		0x00000004, 0x00000010, 0x00000200, 0x00008000,
		0x00020000, 0x00400000, 0x08000000, 0x10000000,
		0x00000002, 0x00000040, 0x00000800, 0x00001000,
		0x00040000, 0x00100000, 0x02000000, 0x80000000};

	/* The key rotation schedule */
static const int	ice_keyrot[16] = {
				0, 1, 2, 3, 2, 1, 3, 0,
				1, 3, 2, 0, 3, 1, 0, 2};


/*
 * 8-bit Galois Field multiplication of a by b, modulo m.
 * Just like arithmetic multiplication, except that additions and
 * subtractions are replaced by XOR.
 */

#ifdef ICEKEY_NEON
static uint32x4_t
gf_mult_x4 (
		uint32x4_t	a,
		uint32x4_t	b,
		uint32x4_t	m
) {
	uint32x4_t	res = vdupq_n_u32(0);
	for(int i = 0; i < 32; ++i)
	{
		// if (b & (1 << i)) res ^= a;
		res = veorq_u32(vandq_u32( vtstq_u32(b, vdupq_n_u32(1 << i)), a), res);
		// a <<= 1;
		a = vshlq_n_u32(a, 1);
		// if (a >= 256) a ^= m;
		a = vbslq_f32(vcgeq_u32(a, vdupq_n_u32(256)), veorq_u32(a, m), a);
	}
	return res;
}
#else
static unsigned int
gf_mult (
	unsigned int	a,
	unsigned int	b,
	unsigned int	m
) {
	unsigned int	res = 0;

	while (b) {
	    if (b & 1)
		res ^= a;

	    a <<= 1;
	    b >>= 1;

	    if (a >= 256)
		a ^= m;
	}

	return (res);
}
#endif

/*
 * Galois Field exponentiation.
 * Raise the base to the power of 7, modulo m.
 */

#ifdef ICEKEY_NEON
static uint32x4_t
gf_exp7_x4 (
		uint32x4_t	b,
		uint32x4_t	m
) {
	uint32x4_t	x;

	x = gf_mult_x4 (b, b, m);
	x = gf_mult_x4 (b, x, m);
	x = gf_mult_x4 (x, x, m);
	x = gf_mult_x4 (b, x, m);

	return vbslq_f32(vceqzq_u32(b),  vdupq_n_u32(0), x);
}
#else
static uint32_t
gf_exp7 (
	unsigned int	b,
	unsigned int		m
) {
	unsigned int	x;

	if (b == 0)
	    return (0);

	x = gf_mult (b, b, m);
	x = gf_mult (b, x, m);
	x = gf_mult (x, x, m);
	return (gf_mult (b, x, m));
}
#endif

/*
 * Carry out the ICE 32-bit P-box permutation.
 */
#ifdef ICEKEY_NEON
static uint32x4_t
ice_perm32_x4 (
		uint32x4_t	x
) {
	uint32x4_t		res = vdupq_n_u32(0);
	for(int i = 0; i < 32; ++i)
	{
		// if (x & (1 << i)) res |= ice_pbox[i];
		res = vorrq_u32(res, vandq_u32(vtstq_u32(x, vdupq_n_u32(1 << i)), vdupq_n_u32(ice_pbox[i])));
	}
	return res;
}
#else
static uint32_t
ice_perm32 (
		uint32_t	x
) {
	uint32_t		res = 0;
	const uint32_t	*pbox = ice_pbox;

	while (x) {
	    if (x & 1)
		res |= *pbox;
	    pbox++;
	    x >>= 1;
	}

	return (res);
}
#endif

/*
 * Initialise the ICE S-boxes.
 * This only has to be done once.
 */

static void
ice_sboxes_init (void)
{
	int	i;
#ifdef ICEKEY_NEON
	const uint32x4x4_t ice_sxor_t = vld4q_u32(&ice_sxor[0][0]);
	const uint32x4x4_t ice_smod_t = vld4q_u32(&ice_smod[0][0]);
	const uint32_t expo_c[4] = { 24, 16, 8, 0 };
	const uint32x4_t expo = vld1q_u32(expo_c);
#endif

	for (i=0; i<1024; i++) {
	    int			col = (i >> 1) & 0xff;
	    int			row = (i & 0x1) | ((i & 0x200) >> 8);
#ifdef ICEKEY_NEON
		uint32x4_t b = veorq_u32(ice_sxor_t.val[row], vdupq_n_u32(col));
		uint32x4_t m = ice_smod_t.val[row];
		uint32x4_t val = gf_exp7_x4(b, m);
		val = vshlq_u32(val, expo);
		ice_sbox[i] = ice_perm32_x4(val);
#else
	    unsigned long	x;

	    x = gf_exp7 (col ^ ice_sxor[0][row], ice_smod[0][row]) << 24;
	    ice_sbox[0][i] = ice_perm32 (x);

	    x = gf_exp7 (col ^ ice_sxor[1][row], ice_smod[1][row]) << 16;
	    ice_sbox[1][i] = ice_perm32 (x);

	    x = gf_exp7 (col ^ ice_sxor[2][row], ice_smod[2][row]) << 8;
	    ice_sbox[2][i] = ice_perm32 (x);

	    x = gf_exp7 (col ^ ice_sxor[3][row], ice_smod[3][row]);
	    ice_sbox[3][i] = ice_perm32 (x);
#endif
	}
}


/*
 * Create a new ICE key.
 */

IceKey::IceKey (int n)
{
	if (!ice_sboxes_initialised) {
	    ice_sboxes_init ();
	    ice_sboxes_initialised = true;
	}

	if (n < 1) {
	    _size = 1;
	    _rounds = 8;
	} else {
	    _size = n;
	    _rounds = n * 16;
	}

	_keysched = new IceSubkey[_rounds];
}


/*
 * Destroy an ICE key.
 */

IceKey::~IceKey ()
{
	int	i, j;

	for (i=0; i<_rounds; i++)
	    for (j=0; j<3; j++)
		_keysched[i].val[j] = 0;

	_rounds = _size = 0;

	delete[] _keysched;
}


/*
 * The single round ICE f function.
 */

static uint32_t
ice_f (
		uint32_t	p,
	const IceSubkey		*sk
) {
	uint32_t	tl, tr;		/* Expanded 40-bit values */
	uint32_t	al, ar;		/* Salted expanded 40-bit values */

					/* Left half expansion */
	tl = ((p >> 16) & 0x3ff) | (((p >> 14) | (p << 18)) & 0xffc00);

					/* Right half expansion */
	tr = (p & 0x3ff) | ((p << 2) & 0xffc00);

					/* Perform the salt permutation */
			// al = (tr & sk->val[2]) | (tl & ~sk->val[2]);
			// ar = (tl & sk->val[2]) | (tr & ~sk->val[2]);
	al = sk->val[2] & (tl ^ tr);
	ar = al ^ tr;
	al ^= tl;

	al ^= sk->val[0];		/* XOR with the subkey */
	ar ^= sk->val[1];

					/* S-box lookup and permutation */
#ifdef ICEKEY_NEON
	auto ret0 = vgetq_lane_u32(ice_sbox[al >> 10], 0);
	auto ret2 = vgetq_lane_u32(ice_sbox[ar >> 10], 2);
	auto ret5 = vgetq_lane_u32(ice_sbox[al & 0x3ff], 1);
	auto ret7 = vgetq_lane_u32(ice_sbox[ar & 0x3ff], 3);
	return ret0 | ret2 | ret5 | ret7;
#else
	return (ice_sbox[0][al >> 10] | ice_sbox[1][al & 0x3ff]
		| ice_sbox[2][ar >> 10] | ice_sbox[3][ar & 0x3ff]);
#endif
}

#ifdef ICEKEY_NEON
static uint32x4_t
ice_f_x4 (
		uint32x4_t	p,
		const IceSubkey		*sk
) {
	uint32x4_t	tl, tr;		/* Expanded 40-bit values */
	uint32x4_t	al, ar;		/* Salted expanded 40-bit values */

	/* Left half expansion */
	tl = ((p >> 16) & 0x3ff) | (((p >> 14) | (p << 18)) & 0xffc00);

	/* Right half expansion */
	tr = (p & 0x3ff) | ((p << 2) & 0xffc00);

	/* Perform the salt permutation */
	// al = (tr & sk->val[2]) | (tl & ~sk->val[2]);
	// ar = (tl & sk->val[2]) | (tr & ~sk->val[2]);
	al = sk->val[2] & (tl ^ tr);
	ar = al ^ tr;
	al ^= tl;

	al ^= sk->val[0];		/* XOR with the subkey */
	ar ^= sk->val[1];

	/* S-box lookup and permutation */
	uint32x4_t result = vdupq_n_u32(0);
#define SET_RESULT(i) result = vsetq_lane_u32( \
			vgetq_lane_u32(ice_sbox[vgetq_lane_u32(al, i) >> 10], 0) |  \
			vgetq_lane_u32(ice_sbox[vgetq_lane_u32(ar, i) >> 10], 2) |  \
			vgetq_lane_u32(ice_sbox[vgetq_lane_u32(al, i) & 0x3ff], 1) |  \
			vgetq_lane_u32(ice_sbox[vgetq_lane_u32(ar, i) & 0x3ff], 3),  \
			result, i)
	SET_RESULT(0);
	SET_RESULT(1);
	SET_RESULT(2);
	SET_RESULT(3);
	return result;
}
#endif


/*
 * Encrypt a block of 8 bytes of data with the given ICE key.
 */

void
IceKey::encrypt (
	const unsigned char	*ptext,
	unsigned char		*ctext
) const
{
	uint32_t l = (((uint32_t) ptext[0]) << 24)
				| (((uint32_t) ptext[1]) << 16)
				| (((uint32_t) ptext[2]) << 8) | ptext[3];
	uint32_t r = (((uint32_t) ptext[4]) << 24)
				| (((uint32_t) ptext[5]) << 16)
				| (((uint32_t) ptext[6]) << 8) | ptext[7];

	for (int i = 0; i < _rounds; i += 2) {
	    l ^= ice_f (r, &_keysched[i]);
	    r ^= ice_f (l, &_keysched[i + 1]);
	}

	for (int i = 0; i < 4; i++) {
	    ctext[3 - i] = r & 0xff;
	    ctext[7 - i] = l & 0xff;

	    r >>= 8;
	    l >>= 8;
	}
}


/*
 * Decrypt a block of 8 bytes of data with the given ICE key.
 */

void
IceKey::decrypt (
	const unsigned char	*ctext,
	unsigned char		*ptext
) const
{
	uint32_t l = (((uint32_t) ctext[0]) << 24)
				| (((uint32_t) ctext[1]) << 16)
				| (((uint32_t) ctext[2]) << 8) | ctext[3];
	uint32_t r = (((uint32_t) ctext[4]) << 24)
				| (((uint32_t) ctext[5]) << 16)
				| (((uint32_t) ctext[6]) << 8) | ctext[7];

	for (int i = _rounds - 1; i > 0; i -= 2) {
	    l ^= ice_f (r, &_keysched[i]);
	    r ^= ice_f (l, &_keysched[i - 1]);
	}

	for (int i = 0; i < 4; i++) {
	    ptext[3 - i] = r & 0xff;
	    ptext[7 - i] = l & 0xff;

	    r >>= 8;
	    l >>= 8;
	}
}

void
IceKey::encrypt32 (const unsigned char *ptext,
                       unsigned char *ctext) const
{
#ifdef ICEKEY_NEON
	uint32x4x2_t ctext32= vld2q_u32((uint32_t *)ctext);
#ifdef __LITTLE_ENDIAN__
	ctext32.val[0] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ctext32.val[0])));
	ctext32.val[1] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ctext32.val[1])));
#endif
	uint32x4_t l = ctext32.val[0];
	uint32x4_t r = ctext32.val[1];

	for (int i = 0; i < _rounds; i += 2) {
		l = veorq_u32(l, ice_f_x4 (r, &_keysched[i]));
		r = veorq_u32(r, ice_f_x4 (l, &_keysched[i + 1]));
	}

	uint32x4x2_t ptext32;
	ptext32.val[0] = r;
	ptext32.val[1] = l;
#ifdef __LITTLE_ENDIAN__
	ptext32.val[0] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ptext32.val[0])));
	ptext32.val[1] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ptext32.val[1])));
#endif
	vst2q_u32((uint32_t *)ptext, ptext32);
#else
	encrypt( ptext, ctext );
	encrypt( ptext + 8, ctext + 8 );
	encrypt( ptext + 16, ctext + 16 );
	encrypt( ptext + 24, ctext + 24 );
#endif
}

void
IceKey::decrypt32 (
		const unsigned char	*ctext,
		unsigned char		*ptext
) const
{
#ifdef ICEKEY_NEON
	uint32x4x2_t ctext32= vld2q_u32((uint32_t *)ctext);
#ifdef __LITTLE_ENDIAN__
	ctext32.val[0] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ctext32.val[0])));
	ctext32.val[1] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ctext32.val[1])));
#endif
	uint32x4_t l = ctext32.val[0];
	uint32x4_t r = ctext32.val[1];

	for (int i = _rounds - 1; i > 0; i -= 2) {
	    l = veorq_u32(l, ice_f_x4 (r, &_keysched[i]));
	    r = veorq_u32(r, ice_f_x4 (l, &_keysched[i - 1]));
	}

	uint32x4x2_t ptext32;
	ptext32.val[0] = r;
	ptext32.val[1] = l;
#ifdef __LITTLE_ENDIAN__
	ptext32.val[0] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ptext32.val[0])));
	ptext32.val[1] = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(ptext32.val[1])));
#endif
	vst2q_u32((uint32_t *)ptext, ptext32);
#else
	decrypt( ctext, ptext );
	decrypt( ctext + 8, ptext + 8 );
	decrypt( ctext + 16, ptext + 16 );
	decrypt( ctext + 24, ptext + 24 );
#endif
}


/*
 * Set 8 rounds [n, n+7] of the key schedule of an ICE key.
 */

void
IceKey::scheduleBuild (
	unsigned short	*kb,
	int		n,
	const int	*keyrot
) {
	int		i;

	for (i=0; i<8; i++) {
	    int	j;
	    int	kr = keyrot[i];
	    IceSubkey		*isk = &_keysched[n + i];

	    for (j=0; j<3; j++)
		isk->val[j] = 0;

	    for (j=0; j<15; j++) {
		int	k;
	    uint32_t	*curr_sk = &isk->val[j % 3];

		for (k=0; k<4; k++) {
		    unsigned short	*curr_kb = &kb[(kr + k) & 3];
		    int	bit = *curr_kb & 1;

		    *curr_sk = (*curr_sk << 1) | bit;
		    *curr_kb = (*curr_kb >> 1) | ((bit ^ 1) << 15);
		}
	    }
	}
}


/*
 * Set the key schedule of an ICE key.
 */

void
IceKey::set (
	const unsigned char	*key
) {
	int		i;

	if (_rounds == 8) {
	    unsigned short	kb[4];

	    for (i=0; i<4; i++)
		kb[3 - i] = (key[i*2] << 8) | key[i*2 + 1];

	    scheduleBuild (kb, 0, ice_keyrot);
	    return;
	}

	for (i=0; i<_size; i++) {
	    int			j;
	    unsigned short	kb[4];

	    for (j=0; j<4; j++)
		kb[3 - j] = (key[i*8 + j*2] << 8) | key[i*8 + j*2 + 1];

	    scheduleBuild (kb, i*8, ice_keyrot);
	    scheduleBuild (kb, _rounds - 8 - i*8, &ice_keyrot[8]);
	}
}


/*
 * Return the key size, in bytes.
 */

int
IceKey::keySize () const
{
	return (_size * 8);
}


/*
 * Return the block size, in bytes.
 */

int
IceKey::blockSize () const
{
	return (8);
}

#endif // !_STATIC_LINKED || _SHARED_LIB
