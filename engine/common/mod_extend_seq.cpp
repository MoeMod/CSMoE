
#include "mod_extend_seq.h"
#include "studio.h"

#define FILE_BUFF_SIZE		2048

typedef struct
{
	int numseq;
	int seqindex;
}
studioseqpack_t;

static void Mod_StudioSeqPackBigEndian(studioseqpack_t* pack)
{
	LittleLongSW(pack->numseq);
	LittleLongSW(pack->seqindex);
}

#define SWAP_INTS(type, startfield) \
for( i = offsetof(type, startfield)/4; i < sizeof(type)/4; i++ ) \
	LittleLongSW(ibuffer[i])

static void Mod_StudioSeqDescBigEndian(mstudioseqdesc_t* pseqdesc)
{
	int* ibuffer = (int*)pseqdesc;
	int i;
	SWAP_INTS(mstudioseqdesc_t, fps);
}

static void Mod_StudioPivotBigEndian(mstudiopivot_t* pivot)
{
	int* ibuffer = (int*)pivot;
	int i;
	SWAP_INTS(mstudiopivot_t, org);
}

#define MAX_SEQUENCEPACKS 256

file_t* g_hSequencePack[MAX_SEQUENCEPACKS];
fs_offset_t g_nSequenceSize[MAX_SEQUENCEPACKS];
int g_iSequenceNums = 0;

byte *Mod_LoadExtendSeq(const char *mod_name, byte *buffer)
{
	// flip to native endian
#ifdef XASH_BIG_ENDIAN
	Mod_StudioBigEndian(NULL, buffer);
#endif
	studiohdr_t *studiohdr = reinterpret_cast<studiohdr_t *>(buffer);

	char newname[PATH_MAX];
	strcpy(newname, mod_name);
	newname[strlen(newname) - 4] = 0;

	for (int i = 0; i < MAX_SEQUENCEPACKS; i++)
	{
		char filename[PATH_MAX];
		sprintf(filename, "%s%d.seq", newname, i + 1);
		g_hSequencePack[g_iSequenceNums] = FS_Open(filename, "rb", false);
		if (!g_hSequencePack[g_iSequenceNums])
			break;

		g_nSequenceSize[g_iSequenceNums] = FS_FileLength(g_hSequencePack[i]);
		g_iSequenceNums++;
	}

	if (g_iSequenceNums > 0)
	{
		int totalsize = 0;

		for (int i = 0; i < g_iSequenceNums; i++)
			totalsize += g_nSequenceSize[i];

		if (totalsize)
		{
			static byte newbuf[0x5000000];
			memset(newbuf, 0, sizeof(newbuf));
			memcpy(newbuf, buffer, LittleLong(studiohdr->length));
			buffer = newbuf;
			studiohdr = reinterpret_cast<studiohdr_t*>(buffer);
		}

		mstudiotexture_t* ptexture = (mstudiotexture_t*)((byte*)studiohdr + studiohdr->textureindex);
		int texdatasize = 0;

		for (int i = 0; i < studiohdr->numtextures; i++)
			texdatasize += ptexture[i].width * ptexture[i].height + 256 * 3;

		mstudiotexture_t* texbuf = (mstudiotexture_t*)Mem_Alloc( com_studiocache, studiohdr->numtextures * sizeof(mstudiotexture_t));
		memcpy(texbuf, ptexture, studiohdr->numtextures * sizeof(mstudiotexture_t));

		byte* texdatabuf = (byte*)Mem_Alloc( com_studiocache, texdatasize);
		memcpy(texdatabuf, (byte*)studiohdr + studiohdr->texturedataindex, texdatasize);

		int offset = 0;
		int endpos = studiohdr->texturedataindex;
		mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)studiohdr + studiohdr->seqindex);

		static byte seqdescbuf[0x2C000];
		memcpy(seqdescbuf, pseqdesc, sizeof(mstudioseqdesc_t) * studiohdr->numseq);

		int numseq = studiohdr->numseq;
		byte* endbuf = (byte*)studiohdr + endpos;
		byte* newbuf = seqdescbuf + (sizeof(mstudioseqdesc_t) * studiohdr->numseq);

		for (int i = 0; i < g_iSequenceNums; i++)
		{
			offset += sizeof(studioseqpack_t);

			if (FS_Read(g_hSequencePack[i], endbuf, g_nSequenceSize[i]) == -1)
			{
				FS_Close(g_hSequencePack[i]);
				g_hSequencePack[i] = nullptr;
				continue;
			}

			studioseqpack_t* pack = (studioseqpack_t*)endbuf;
#ifdef XASH_BIG_ENDIAN
			Mod_StudioSeqPackBigEndian(pack);
#endif
			mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)(endbuf + pack->seqindex);

			for (int j = 0; j < pack->numseq; j++)
			{
#ifdef XASH_BIG_ENDIAN
				Mod_StudioSeqDescBigEndian(&pseqdesc[j]);
#endif
			}

			for (int j = 0; j < pack->numseq; j++)
			{
				pseqdesc[j].eventindex = endpos + offset;
				offset += pseqdesc[j].numevents * sizeof(mstudioevent_t);
				pseqdesc[j].pivotindex = endpos + offset;
				offset += pseqdesc[j].numpivots * sizeof(mstudiopivot_t);
			}

			for (int j = 0; j < pack->numseq; j++)
			{
				int animindex = endpos + offset;

				if (j == pack->numseq - 1)
					offset = (byte*)pseqdesc - endpos - (byte*)studiohdr;
				else
					offset += pseqdesc[j + 1].animindex - pseqdesc[j].animindex;

				pseqdesc[j].animindex = animindex;
			}

			memcpy(newbuf, pseqdesc, sizeof(mstudioseqdesc_t) * pack->numseq);
			newbuf += sizeof(mstudioseqdesc_t) * pack->numseq;
			numseq += pack->numseq;
			endbuf += pack->seqindex;

			FS_Close(g_hSequencePack[i]);
		}

		memcpy(endbuf, seqdescbuf, sizeof(mstudioseqdesc_t) * numseq);

		studiohdr->numseq = numseq;
		studiohdr->seqindex = endbuf - (byte*)studiohdr;

		endbuf += sizeof(mstudioseqdesc_t) * numseq;
		memcpy(endbuf, texbuf, sizeof(mstudiotexture_t) * studiohdr->numtextures);

		ptexture = (mstudiotexture_t*)endbuf;
		studiohdr->textureindex = endbuf - (byte*)studiohdr;
		endbuf += sizeof(mstudiotexture_t) * studiohdr->numtextures;
		texdatasize = 0;

		for (int i = 0; i < studiohdr->numtextures; i++)
		{
			ptexture[i].index = (endbuf + texdatasize) - (byte*)studiohdr;
			texdatasize += ptexture[i].width * ptexture[i].height + 256 * 3;
		}

		memcpy(endbuf, texdatabuf, texdatasize);

		studiohdr->texturedataindex = endbuf - (byte*)studiohdr;
		endbuf += texdatasize;
		studiohdr->length = endbuf - (byte*)studiohdr;

		Mem_Free(texbuf);
		Mem_Free(texdatabuf);

		g_iSequenceNums = 0;
	}
	// flip back to file endian
#ifdef XASH_BIG_ENDIAN
	Mod_StudioBigEndian(NULL, buffer);
#endif
	return buffer;
}