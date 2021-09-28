#include "mod_extend_seq.h"
#include "studio.h"

#define FILE_BUFF_SIZE		2048

typedef struct
{
	int numseq;
	int seqindex;
}
studioseqpack_t;

struct file_s
{
	int		handle;			// file descriptor
	fs_offset_t	real_length;		// uncompressed file size (for files opened in "read" mode)
	fs_offset_t	position;			// current position in the file
	fs_offset_t	offset;			// offset into the package (0 if external file)
	int		ungetc;			// single stored character from ungetc, cleared to EOF when read
	time_t		filetime;			// pak, wad or real filetime
						// Contents buffer
	fs_offset_t	buff_ind, buff_len;		// buffer current index and length
	byte		buff[FILE_BUFF_SIZE];	// intermediate buffer
};

#define MAX_SEQUENCEPACKS 256

file_t* g_hSequencePack[MAX_SEQUENCEPACKS];
fs_offset_t g_nSequenceSize[MAX_SEQUENCEPACKS];
int g_iSequenceNums = 0;

extern "C" byte *Mod_LoadExtendSeq(model_t *mod, byte *buffer)
{
	studiohdr_t *studiohdr = reinterpret_cast<studiohdr_t *>(buffer);

	char newname[64];
	strcpy(newname, mod->name);
	newname[strlen(newname) - 4] = 0;

	for (int i = 0; i < MAX_SEQUENCEPACKS; i++)
	{
		char filename[64];
		sprintf(filename, "%s%d.seq", newname, i + 1);
		g_hSequencePack[g_iSequenceNums] = FS_Open(filename, "rb", false);
		if (!g_hSequencePack[g_iSequenceNums])
			break;

		g_nSequenceSize[g_iSequenceNums] = g_hSequencePack[g_iSequenceNums]->real_length;
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
			studiohdr = (studiohdr_t*)newbuf;
		}

		mstudiotexture_t* ptexture = (mstudiotexture_t*)((byte*)studiohdr + studiohdr->textureindex);
		int texdatasize = 0;

		for (int i = 0; i < studiohdr->numtextures; i++)
			texdatasize += ptexture[i].width * ptexture[i].height + 256 * 3;

		mstudiotexture_t* texbuf = (mstudiotexture_t*)malloc(studiohdr->numtextures * sizeof(mstudiotexture_t));
		memcpy(texbuf, ptexture, studiohdr->numtextures * sizeof(mstudiotexture_t));

		byte* texdatabuf = (byte*)malloc(texdatasize);
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
				continue;
			}

			studioseqpack_t* pack = (studioseqpack_t*)endbuf;
			mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)(endbuf + pack->seqindex);

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

		free(texbuf);
		free(texdatabuf);

		g_iSequenceNums = 0;
	}
	return buffer;
}