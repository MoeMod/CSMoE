#include "mod_decryptor.h"
#include "port.h"
#include "IceKey.hpp"
#include "studio.h"

static const byte g_pDecryptorKey_20[32] =
{
	0x32, 0xA6, 0x21, 0xE0, 0xAB, 0x6B, 0xF4, 0x2C,
	0x93, 0xC6, 0xF1, 0x96, 0xFB, 0x38, 0x75, 0x68,
	0xBA, 0x70, 0x13, 0x86, 0xE0, 0xB3, 0x71, 0xF4,
	0xE3, 0x9B, 0x07, 0x22, 0x0C, 0xFE, 0x88, 0x3A
};

static const byte g_pDecryptorKey_21[32] =
{
	0x22, 0x7A, 0x19, 0x6F, 0x7B, 0x86, 0x7D, 0xE0,
	0x8C, 0xC6, 0xF1, 0x96, 0xFB, 0x38, 0x75, 0x68,
	0x88, 0x7A, 0x78, 0x86, 0x78, 0x86, 0x67, 0x70,
	0xD9, 0x91, 0x07, 0x3A, 0x14, 0x74, 0xFE, 0x22
};

CIceKey g_Decryptor(4);

void DecryptChunk(byte *pData, size_t uDataSize)
{
	if (!uDataSize)
		return;

	size_t uCount = (uDataSize + 7) >> 3;

	while (uCount)
	{
		g_Decryptor.Decrypt(pData, pData);
		pData += 8;
		uCount--;
	}
}

void DecryptData(byte *pData, size_t uDataSize)
{
	if (!uDataSize)
		return;

	do
	{
		size_t uTempSize = uDataSize;

		if (uTempSize > 1024)
			uTempSize = 1024;

		if (uTempSize & 7)
			return;

		DecryptChunk(pData, uTempSize);
		pData += uTempSize;
		uDataSize -= uTempSize;
	} while (uDataSize);
}

extern "C" void Mod_DecryptModel(model_t *mod, byte *buffer)
{
	studiohdr_t *studiohdr = reinterpret_cast<studiohdr_t *>(buffer);

	if (!Q_strncmp(mod->name, "models/player", 13))
	{
		if (studiohdr->numhitboxes == 21)
			studiohdr->numhitboxes = 20;
	}

	if (studiohdr->version == 20 || studiohdr->version == 21)
	{
		if (studiohdr->version == 20)
			g_Decryptor.SetKey(g_pDecryptorKey_20);
		else if (studiohdr->version == 21)
			g_Decryptor.SetKey(g_pDecryptorKey_21);

		mstudiotexture_t *ptexture = (mstudiotexture_t *)(buffer + studiohdr->textureindex);

		for (int i = 0; i < studiohdr->numtextures; i++)
			DecryptData(buffer + ptexture[i].index, (ptexture[i].width * ptexture[i].height) + (256 * 3));

		mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)(buffer + studiohdr->bodypartindex);

		for (int i = 0; i < studiohdr->numbodyparts; i++)
		{
			mstudiomodel_t *pmodel = (mstudiomodel_t *)(buffer + pbodypart[i].modelindex);

			for (int j = 0; j < pbodypart[i].nummodels; j++)
			{
				if (pmodel[j].numverts > 0)
					DecryptData(buffer + pmodel[j].vertindex, pmodel[j].numverts * sizeof(vec3_t));
			}
		}

		studiohdr->version = 10;
	}
}