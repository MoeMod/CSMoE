#include "hud.h"
#include "triangleapi.h"
#include "r_efx.h"
#include "client.h"
#include "cl_util.h"


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "SpritePlayer.h"

namespace cl {


	void CSpritePlayer::Init(char* szSprite, float framerate)
	{
		if (!szSprite)
			return;

		HSPRITE hSprite = gEngfuncs.pfnSPR_Load(szSprite);
		if (!hSprite)
		{
			if (m_Sprite.name[0])
				memset(&m_Sprite, 0, sizeof(m_Sprite));

			return;
		}

		strcpy(m_Sprite.name, szSprite);
		m_Sprite.model = (model_t*)gEngfuncs.GetSpritePointer(hSprite);

		m_Sprite.width = MAX(gEngfuncs.pfnSPR_Width(hSprite, 0), 0);
		m_Sprite.height = MAX(gEngfuncs.pfnSPR_Height(hSprite, 0), 0);

		m_Sprite.frame = 0;
		m_Sprite.maxframe = m_Sprite.model->numframes - 1;
		
		m_Sprite.framerate = framerate;

		float fps = 60.0f;
		if (m_Sprite.framerate)
			fps *= m_Sprite.framerate;

		m_Sprite.playtime = (float)m_Sprite.maxframe / fps;

		m_iFlag = FLAG_NONE;
		m_iReverse = REVERSE_NONE;
		m_iRendermode = kRenderNormal;
		m_iX = 0;
		m_iY = 0;
		m_iWidth = m_iWidth2 = m_Sprite.width;
		m_iHeight = m_iHeight2 = m_Sprite.height;
		m_iR = m_iG = m_iB = m_iAlpha = 255;
		m_bPlaying = false;
		m_flDisplayEndTime = 0.0f;
		m_flDisplayTime = 0.0f;
	}

	void CSpritePlayer::SetFlag(int iFlag)
	{
		if (iFlag == FLAG_NONE)
		{
			m_iFlag = FLAG_NONE;
			return;
		}

		m_iFlag |= (1 << iFlag);
	}

	void CSpritePlayer::Play(float time)
	{
		if (m_iFlag & (1 << FLAG_LOOP) && m_iFlag & (1 << FLAG_NONSTOP) && m_bPlaying)
			return;

		m_bPlaying = true;
		m_flDisplayTime = gEngfuncs.GetClientTime();
		m_flLastFrameTime = gEngfuncs.GetClientTime();

		if (time == -1.0)
			m_flDisplayEndTime = cl.time + m_Sprite.playtime;
		else
			m_flDisplayEndTime = cl.time + time;

		if (m_iFlag & (1 << FLAG_REVERSE))
			m_Sprite.frame = m_Sprite.maxframe;
		else
			m_Sprite.frame = 0;
	}

	void CSpritePlayer::Paint(float time)
	{
		if (!m_Sprite.model || !m_bPlaying)
			return;

		if (m_flDisplayEndTime <= cl.time)
		{
			if (!(m_iFlag & (1 << FLAG_NONSTOP)))
			{
				m_bPlaying = false;
				return;
			}
		}

		//glEnable(GL_TEXTURE_2D);
		gEngfuncs.pTriAPI->RenderMode(m_iRendermode);
		//gEngfuncs.pTriAPI->Begin(GL_TEXTURE_2D);


		//glColor4ub(m_iR, m_iG, m_iB, m_iAlpha);
		gEngfuncs.pTriAPI->Color4ub(m_iR, m_iG, m_iB, m_iAlpha);

		gEngfuncs.pTriAPI->SpriteTexture(m_Sprite.model, m_Sprite.frame);
		gEngfuncs.pTriAPI->Begin(TRI_QUADS);

		static int iW, iH;
		iW = m_iWidth2;
		iH = m_iHeight2;

		switch (m_iReverse)
		{
		case 1:
			iW *= -1;
			break;
		case 2:
			iH *= -1;
			break;
		case 3:
		{
			iW *= -1;
			iH *= -1;
			break;
		}
		}

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f(m_iX + iW, m_iY - iH, 0);
		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(m_iX - iW, m_iY - iH, 0);
		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(m_iX - iW, m_iY + iH, 0);
		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f(m_iX + iW, m_iY + iH, 0);
		gEngfuncs.pTriAPI->End();

		if (m_Sprite.maxframe)
		{
			if (m_iFlag & (1 << FLAG_REVERSE))
			{
				if (m_Sprite.frame <= 0)
				{
					if (m_iFlag & (1 << FLAG_LOOP))
					{
						m_Sprite.frame = m_Sprite.maxframe;
						m_flDisplayTime = gEngfuncs.GetClientTime();
					}
					else if (m_iFlag & (1 << FLAG_FORCE_STOP))
					{
						m_bPlaying = false;
						return;
					}
				}
				else if (!(m_iFlag & (1 << FLAG_NOFRAME)))
				{
					float time = gEngfuncs.GetClientTime();

					if (1.0 / (60.0 * m_Sprite.framerate) <= time - m_flLastFrameTime)
					{
						m_Sprite.frame--;
						m_flLastFrameTime = time;
					}
				}
			}
			else
			{
				if (m_Sprite.frame >= m_Sprite.maxframe)
				{
					if (m_iFlag & (1 << FLAG_LOOP))
					{
						m_Sprite.frame = 0;
						m_flDisplayTime = gEngfuncs.GetClientTime();
					}
					else if (m_iFlag & (1 << FLAG_FORCE_STOP))
					{
						m_bPlaying = false;
						return;
					}
				}
				else if (!(m_iFlag & (1 << FLAG_NOFRAME)))
				{
					float time = gEngfuncs.GetClientTime();

					if (1.0 / (60.0 * m_Sprite.framerate) <= time - m_flLastFrameTime)
					{
						m_Sprite.frame++;
						m_flLastFrameTime = time;
					}
				}
			}
		}
	}
}