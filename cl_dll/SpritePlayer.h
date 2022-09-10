#ifndef SPRITEPLAYER_H
#define SPRITEPLAYER_H

#if defined _WIN32
#pragma once
#endif

namespace cl {
	enum SpritePlayer_Flag
	{
		FLAG_NONE = 0,
		FLAG_NOFRAME,
		FLAG_LOOP,
		FLAG_FORCE_STOP,
		FLAG_NONSTOP,
		FLAG_REVERSE
	};

	enum SpritePlayer_Reverse
	{
		REVERSE_NONE = 0,
		REVERSE_HORIZON,
		REVERSE_VERTICAL,
		REVERSE_ALL
	};

	class CSpritePlayer
	{
		typedef struct Spritedata_s
		{
			char name[64];
			model_t* model;
			int width;
			int height;
			int frame;
			int maxframe;
			float framerate; // fps = 60 * framerate
			float playtime;
		}
		Spritedata_t;

	public:
		void Init(char* szSprite, float framerate);
		void SetPos(int x, int y)
		{
			m_iX = x;
			m_iY = y;
		};
		void SetPos(float* p)
		{
			if (!p)
				return;

			m_iX = p[0];
			m_iY = p[1];
		};
		void SetSize(int width, int height)
		{
			m_iWidth2 = width;
			m_iHeight2 = height;
		};
		void SetColor(int r, int g, int b, int a)
		{
			m_iR = r;
			m_iG = b;
			m_iB = b;
			m_iAlpha = a;
		};
		void SetRendermode(int mode) { m_iRendermode = mode; };
		void SetFlag(int iFlag);
		void Play(float time = 0.0f);
		void Stop(void) { m_bPlaying = false; };
		bool IsPlaying(void) { return m_bPlaying ? true : false; };
		bool IsValid(void) { return m_Sprite.model ? true : false; };
		void Paint(float time);
		void ResetSize(void)
		{
			m_iWidth2 = m_iWidth;
			m_iHeight2 = m_iHeight;
		}
		void SetReverse(int iReverse) { m_iReverse = iReverse; };
		int GetMaxFrame(void) { return m_Sprite.maxframe; };
		void SetFrame(int iFrame) { m_Sprite.frame = iFrame; };
		int GetFrame(void) { return m_Sprite.frame; };

		int m_iWidth2, m_iHeight2;
	private:
		Spritedata_t m_Sprite;

		int m_iFlag;

		int m_iX, m_iY;
		int m_iWidth, m_iHeight;
		int m_iR, m_iG, m_iB, m_iAlpha;
		int m_iRendermode;

		bool m_bPlaying;
		int m_iReverse;
		float m_flDisplayTime;
		float m_flDisplayEndTime;
		float m_flLastFrameTime;
	};
}
#endif