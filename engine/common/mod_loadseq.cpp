#include "mod_loadseq.h"
#include "port.h"
#include "studio.h"

#define MAX_SEQUENCEPACKS 256
static file_t *g_hSequencePack[MAX_SEQUENCEPACKS];
static size_t g_nSequenceSize[MAX_SEQUENCEPACKS];
static int g_iSequenceNums = 0;

typedef struct
{
	int numseq;
	int seqindex;
}
studioseqpack_t;

extern "C" void Mod_LoadSeq(model_t *mod, byte *buffer)
{
	studiohdr_t *studiohdr = reinterpret_cast<studiohdr_t *>(buffer);

	char newname[MAX_QPATH];
	strcpy(newname, mod->name);
	newname[strlen(newname) - 4] = 0;

	for (int i = 0; i < 256; i++)
	{
		char filename[MAX_QPATH];
		sprintf(filename, "%s%d.seq", newname, i + 1);

		g_hSequencePack[i] = FS_Open(filename, "rb", false);

		if (g_hSequencePack[i] == nullptr)
			break;

		g_nSequenceSize[i] = FS_FileLength(g_hSequencePack[i]);
		g_iSequenceNums++;
	}

	if (g_iSequenceNums > 0)
	{
		int totalsize = 0;

		for (int i = 0; i < g_iSequenceNums; i++)
			totalsize += g_nSequenceSize[i];

		if (totalsize)
		{
			static byte newbuf[0x500000];
			memset(newbuf, 0, sizeof(newbuf));
			memcpy(newbuf, buffer, studiohdr->length);
			buffer = newbuf;
			studiohdr = (studiohdr_t *)newbuf;
		}

		mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)studiohdr + studiohdr->textureindex);
		int texdatasize = 0;

		for (int i = 0; i < studiohdr->numtextures; i++)
			texdatasize += ptexture[i].width * ptexture[i].height + 256 * 3;

		mstudiotexture_t *texbuf = (mstudiotexture_t *)malloc(studiohdr->numtextures * sizeof(mstudiotexture_t));
		memcpy(texbuf, ptexture, studiohdr->numtextures * sizeof(mstudiotexture_t));

		byte *texdatabuf = (byte *)malloc(texdatasize);
		memcpy(texdatabuf, (byte *)studiohdr + studiohdr->texturedataindex, texdatasize);

		int offset = 0;
		int endpos = studiohdr->texturedataindex;
		mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)studiohdr + studiohdr->seqindex);

		static byte seqdescbuf[0x2C000];
		memcpy(seqdescbuf, pseqdesc, sizeof(mstudioseqdesc_t) * studiohdr->numseq);

		int numseq = studiohdr->numseq;
		byte *endbuf = (byte *)studiohdr + endpos;
		byte *newbuf = seqdescbuf + (sizeof(mstudioseqdesc_t) * studiohdr->numseq);

		for (int i = 0; i < g_iSequenceNums; i++)
		{
			offset += sizeof(studioseqpack_t);

			if (FS_Eof(g_hSequencePack[i]))
			{
				FS_Close(g_hSequencePack[i]);
				g_hSequencePack[i] = nullptr;
				continue;
			}

			FS_Read(g_hSequencePack[i], endbuf, g_nSequenceSize[i]);

			studioseqpack_t *pack = (studioseqpack_t *)endbuf;
			mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)(endbuf + pack->seqindex);

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
					offset = (byte *)pseqdesc - endpos - (byte *)studiohdr;
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
		studiohdr->seqindex = endbuf - (byte *)studiohdr;

		endbuf += sizeof(mstudioseqdesc_t) * numseq;
		memcpy(endbuf, texbuf, sizeof(mstudiotexture_t) * studiohdr->numtextures);

		ptexture = (mstudiotexture_t *)endbuf;
		studiohdr->textureindex = endbuf - (byte *)studiohdr;
		endbuf += sizeof(mstudiotexture_t) * studiohdr->numtextures;
		texdatasize = 0;

		for (int i = 0; i < studiohdr->numtextures; i++)
		{
			ptexture[i].index = (endbuf + texdatasize) - (byte *)studiohdr;
			texdatasize += ptexture[i].width * ptexture[i].height + 256 * 3;
		}

		memcpy(endbuf, texdatabuf, texdatasize);

		studiohdr->texturedataindex = endbuf - (byte *)studiohdr;
		endbuf += texdatasize;
		studiohdr->length = endbuf - (byte *)studiohdr;

		free(texbuf);
		free(texdatabuf);

		g_iSequenceNums = 0;
	}
}