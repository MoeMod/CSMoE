
extern int g_infinite;

int CNewHud::Vidinit(void)
{
	m_iCharacterBG = FindTexture("resource\\hud\\hud_character_bg");
	m_iCharacterBG_Bottom = FindTexture("resource\\hud\\hud_character_bg_bottom");
	m_iCharacterBG_Top = FindTexture("resource\\hud\\hud_character_bg_top");
	m_iDollarBG = FindTexture("resource\\hud\\hud_dollar_bg");

	m_iIconHuman = FindTexture("resource\\hud\\hud_image_icon_human");

	m_iLine = FindTexture("resource\\hud\\hud_line");

	m_iIcon_MapPoint = FindTexture("resource\\hud\\hud_map_point_icon");
	m_iMapTitleBG = FindTexture("resource\\hud\\hud_maptitle_bg");

	m_iNum_BigBlue = DrawTgaGetID2("resource\\hud\\hud_sb_num_big_blue");
	m_iNum_BigRed = DrawTgaGetID2("resource\\hud\\hud_sb_num_big_red");

	m_iNum_BigWhite = DrawTgaGetID2("resource\\hud\\hud_sb_num_big_white");
	m_iColon_BigWhite = FindTexture("resource\\hud\\hud_sb_num_big_white_colon");

	m_iNum_Bottom = DrawTgaGetID2("resource\\hud\\hud_sb_num_bottom");
	m_iColon_Bottom = FindTexture("resource\\hud\\hud_sb_num_bottom_colon");

	m_iNum_Center = DrawTgaGetID2("resource\\hud\\hud_sb_num_center");
	m_iSlash_Center = FindTexture("resource\\hud\\hud_sb_num_center_slash");

	m_iPoint_Human = FindTexture("resource\\hud\\hud_sb_num_human_point");
	m_iSlash_Human = FindTexture("resource\\hud\\hud_sb_num_human_slash");

	m_iNum_HumanSmall = DrawTgaGetID2("resource\\hud\\hud_sb_num_human_small");
	m_iNum_HumanScenario = DrawTgaGetID2("resource\\hud\\hud_sb_num_humanscenario");

	m_iNum_SmallBlue = DrawTgaGetID2("resource\\hud\\hud_sb_num_small_blue");
	m_iNum_SmallRed = DrawTgaGetID2("resource\\hud\\hud_sb_num_small_red");

	m_iNum_Character = DrawTgaGetID2("resource\\hud\\hud_character_num");

	char temp[64];
	for (int i = 0; i < 10; i++)
	{
		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_big_blue%d", i);
		m_iNum_BigBlueC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_big_red%d", i);
		m_iNum_BigRedC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_big_white%d", i);
		m_iNum_BigWhiteC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_bottom%d", i);
		m_iNum_BottomC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_center%d", i);
		m_iNum_CenterC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_human_small%d", i);
		m_iNum_HumanSmallC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_humanscenario%d", i);
		m_iNum_HumanScenarioC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_small_blue%d", i);
		m_iNum_SmallBlueC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_sb_num_small_red%d", i);
		m_iNum_SmallRedC[i] = FindTexture(temp);

		sprintf_s(temp, "resource\\hud\\number\\hud_character_num%d", i);
		m_iNum_CharacterC[i] = FindTexture(temp);
	}

	if (!m_iScoreboardBG)
		m_iScoreboardBG = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg");

	if (!m_iScoreboardBG_GD)
		m_iScoreboardBG_GD = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_gundeath");

	if (!m_iScoreboardBG_HMS)
		m_iScoreboardBG_HMS = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_humanscenario");

	if (!m_iScoreboardBG_TIME)
		m_iScoreboardBG_TIME = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_time");

	if (!m_iScoreboardBG_ZB3)
		m_iScoreboardBG_ZB3 = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_zb3");

	if (!m_iScoreboardBG_ZBF)
		m_iScoreboardBG_ZBF = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_zfile_pve");

	if (!m_iScoreboardBG_ZBG)
		m_iScoreboardBG_ZBG = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_zombie-gaint");

	if (!m_iScoreboardBG_ZSHT01)
		m_iScoreboardBG_ZSHT01 = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_zsht_pvp01");

	if (!m_iScoreboardBG_ZSHT02)
		m_iScoreboardBG_ZSHT02 = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_zsht_pvp02");

	if (!m_iScoreboardBG_ZTD01)
		m_iScoreboardBG_ZTD01 = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_ztd01");

	if (!m_iScoreboardBG_ZSHT02)
		m_iScoreboardBG_ZSHT02 = R_LoadTextureUnique("resource/hud/hud_scoreboard_bg_ztd02");


	m_iSkillIconBG = FindTexture("resource\\hud\\hud_skillicon_bg");

	m_iIcon_1st = FindTexture("resource\\hud\\hud_text_icon_1st");

	m_iIcon_A_blueL = FindTexture("resource\\hud\\hud_text_icon_a_left_blue");
	m_iIcon_A_redL = FindTexture("resource\\hud\\hud_text_icon_a_left_red");

	m_iIcon_A_blueR = FindTexture("resource\\hud\\hud_text_icon_a_right_blue");
	m_iIcon_A_redR = FindTexture("resource\\hud\\hud_text_icon_a_right_red");

	m_iIcon_B_blueL = FindTexture("resource\\hud\\hud_text_icon_b_left_blue");
	m_iIcon_B_redL = FindTexture("resource\\hud\\hud_text_icon_b_left_red");

	m_iIcon_B_blueR = FindTexture("resource\\hud\\hud_text_icon_b_right_blue");
	m_iIcon_B_redR = FindTexture("resource\\hud\\hud_text_icon_b_right_red");

	m_iIcon_CT_Left = FindTexture("resource\\hud\\hud_text_icon_ct_left");
	m_iIcon_CT_Right = FindTexture("resource\\hud\\hud_text_icon_ct_right");

	m_iIcon_Goal = FindTexture("resource\\hud\\hud_text_icon_goal_center");

	m_iIcon_HM_Left = FindTexture("resource\\hud\\hud_text_icon_hm_left");
	m_iIcon_HM_Right = FindTexture("resource\\hud\\hud_text_icon_hm_right");

	m_iIcon_Kill = FindTexture("resource\\hud\\hud_text_icon_kill");
	m_iIcon_KillC = FindTexture("resource\\hud\\hud_text_icon_kill_center");
	m_iIcon_KillR = FindTexture("resource\\hud\\hud_text_icon_kill_right");

	m_iIcon_Lv = FindTexture("resource\\hud\\hud_text_icon_level");
	m_iIcon_LvUp = FindTexture("resource\\hud\\hud_text_icon_lv-up");
	m_iIcon_LvMax = FindTexture("resource\\hud\\hud_text_icon_max-lv");

	m_iIcon_My = FindTexture("resource\\hud\\hud_text_icon_my");

	m_iIcon_Round = FindTexture("resource\\hud\\hud_text_icon_round");
	m_iIcon_Score = FindTexture("resource\\hud\\hud_text_icon_score");

	m_iIcon_TeamKill = FindTexture("resource\\hud\\hud_text_icon_team-kill");
	m_iIcon_TimeAttack = FindTexture("resource\\hud\\hud_text_icon_time-attack");
	m_iIcon_TotalKillL = FindTexture("resource\\hud\\hud_text_icon_totalkill_left");

	if (!m_iIcon_TR_Left)
		m_iIcon_TR_Left = R_LoadTextureUnique("resource/hud/hud_text_icon_tr_left");

	if (!m_iIcon_TR_Right)
		m_iIcon_TR_Right = R_LoadTextureUnique("resource/hud/hud_text_icon_tr_right");


	m_iIcon_Win_Center = FindTexture("resource\\hud\\hud_text_icon_win_center");
	m_iIcon_Win_Right = FindTexture("resource\\hud\\hud_text_icon_win_right");

	m_iIcon_ZB_Left = FindTexture("resource\\hud\\hud_text_icon_zb_left");
	m_iIcon_ZB_Right = FindTexture("resource\\hud\\hud_text_icon_zb_right");

	m_iWeaponBG = FindTexture("resource\\hud\\hud_weapon_bg");

	m_iWeapon_OffBG = FindTexture("resource\\hud\\hud_weapon_off_bg");
	m_iWeapon_OnBG = FindTexture("resource\\hud\\hud_weapon_on_bg");

	m_iList_NewWpn = FindTexture("resource\\hud\\weapon_list_new");

	m_iIcon_ZBNotDead = DrawTgaGetID2("resource\\hud\\zombienotdead_icon_new");
	m_iBg_ZBNotDead = FindTexture("resource\\hud\\zombienotdead_icon_bg");

	m_iIcon_ZSHT_Moon = FindTexture("resource\\hud\\zsh_ingame_moon_icon");
	m_iIcon_ZSHT_Sun = FindTexture("resource\\hud\\zsh_ingame_sun_icon");
	m_iIcon_ZSHT_Escape = FindTexture("resource\\hud\\zsht_ingame_escape_icon");

	m_iIcon_Damage = FindTexture("resource\\hud\\hud_icon_damage");
	m_iIcon_Speed = FindTexture("resource\\hud\\hud_icon_speed");

	m_iNvgOff = GetSpriteIndex("nightvision_off_new");
	m_iNvgOn = GetSpriteIndex("nightvision_on_new");
	m_iNvgZBOff = GetSpriteIndex("zombie_icon_new_off");
	m_iNvgZBOn = GetSpriteIndex("zombie_icon_new_on");
	m_iDefuser = GetSpriteIndex("defuser_new");

	m_iDollar = GetSpriteIndex("dollar_new");
	m_iMinus = GetSpriteIndex("minus_new");
	m_iPlus = GetSpriteIndex("plus_new");

	//m_iDivisionline = GetSpriteIndex("divisionline_new");

	m_iWeaponSel = GetSpriteIndex("weapon_selection_new");
	m_iWeaponGet = GetSpriteIndex("weapon_get_bg_new");

	m_iC4On = GetSpriteIndex("c4_on_new");
	m_iC4Off = GetSpriteIndex("c4_off_new");
	m_iHostage = GetSpriteIndex("hostage_new");

	m_iCross = GetSpriteIndex("cross_new");
	m_iSuit[0] = GetSpriteIndex("suit_empty_new");
	m_iSuit[1] = GetSpriteIndex("suit_full_new");
	m_iSuitHelmet[0] = GetSpriteIndex("suithelmet_empty_new");
	m_iSuitHelmet[1] = GetSpriteIndex("suithelmet_full_new");

	m_iDollarNumberZero = GetSpriteIndex("dollarNum_0_new");
	m_iNumberZero = GetSpriteIndex("number_0_new");

	m_iZombieKnife_i = GetSpriteIndex("zombieknife_a");
	m_iZombieKnife_ia = GetSpriteIndex("zombieknife_ia");

	m_iIcon_Flash[0] = GetSpriteIndex("flash_full");
	m_iIcon_Flash[1] = GetSpriteIndex("flash_empty");
	m_iIcon_Flash[2] = GetSpriteIndex("flash_beam");

	m_iBuyzone = GetSpriteIndex("buyzone");

	m_iFontWidth = GetSpriteRect(m_iNumberZero).right - GetSpriteRect(m_iNumberZero).left;
	m_iFontWidth_Dollar = GetSpriteRect(m_iDollarNumberZero).right - GetSpriteRect(m_iDollarNumberZero).left;
	m_iFontHeight = GetSpriteRect(m_iNumberZero).bottom - GetSpriteRect(m_iNumberZero).top;
	m_iFontHeight_Dollar = GetSpriteRect(m_iDollarNumberZero).bottom - GetSpriteRect(m_iDollarNumberZero).top;

	memset(m_AmmoPickup, 0, sizeof(m_AmmoPickup));
	memset(m_WeaponPickup, 0, sizeof(m_WeaponPickup));

	memset(m_szClass, 0, sizeof(m_szClass));

	m_iClass = 0;

	m_bUseFlash = true;

	m_bHasNvg = false;
	m_bUsingNvg = false;

	m_iMoney = 0;
	m_iMoneyChange = 0;
	m_flMoney = 0.0f;

	m_flZBIconFlash = 0.0f;

	m_flTime = 0.0f;

	return 1;
}

void CNewHud::Draw(float flTime)
{
	extern cvar_t* hud_draw;
	if (!hud_draw->value)
		return;

	if (gConfigs.iHudType != 2)
		return;


	DisplayScoreboard(flTime);

	if (!g_bAlive || gEngfuncs.IsSpectateOnly())
		return;

	int iX = 0;
	int iY = g_sScreenInfo.iHeight - 5;

	MH_DrawTGAFunction2(g_Texture[m_iCharacterBG_Bottom].iTexture, iX, iY - g_Texture[m_iCharacterBG_Bottom].iHeight, g_Texture[m_iCharacterBG_Bottom].iWidth, g_Texture[m_iCharacterBG_Bottom].iHeight, 255);

	static int g_iShowPlayerState;
	g_iShowPlayerState = g_ClassData[ClassManager().GetUserClass(gEngfuncs.GetLocalPlayer()->index)].m_iBitsShowState;

	if (g_iShowPlayerState)
		MH_DrawTGAFunction2(g_Texture[m_iCharacterBG_Top].iTexture, iX, iY - g_Texture[m_iCharacterBG_Top].iHeight, g_Texture[m_iCharacterBG_Top].iWidth, g_Texture[m_iCharacterBG_Top].iHeight, 255);

	DisplayPlayerState(flTime);
	if (IS_ZOMBIE_MODE)
	{
		if (g_iExHealth > 0)
			DisplayTimerHealth(flTime, 0);

		DisplayZombieScore();
	}

	iX = g_sScreenInfo.iWidth;
	MH_DrawTGAFunction2(g_Texture[m_iWeaponBG].iTexture, iX - g_Texture[m_iWeaponBG].iWidth, iY - g_Texture[m_iWeaponBG].iHeight, g_Texture[m_iWeaponBG].iWidth, g_Texture[m_iWeaponBG].iHeight, 255);
	DisplayWeapon(flTime);

	DisplayMoney(flTime);
	DisplayWeaponState(flTime);
	DisplayItemState(flTime);
}


void CNewHud::DrawNumber(int n, int x, int y, float scale, int r, int g, int b)
{
	float w = m_iNumber->w() * scale;
	float h = m_iNumber->h() * scale;
	int left = m_rcNumber[n].left * scale;
	int right = m_rcNumber[n].right * scale;
	int bottom = m_rcNumber[n].bottom * scale;
	int top = m_rcNumber[n].top * scale;
	int x2 = x + (right - left);
	int y2 = y + (bottom - top);
	m_iNumber->Draw2DQuadScaled(x, y, x2, y2, left / w, top / h, right / w, bottom / h, 225, 225, 225);
}


void CNewHud::DrawNumbers(int n, int x, int y, int from_right, float scale, int r, int g, int b)
{
	int width = 14;
	width *= scale;

	int k;

	if (n >= 100000)
	{
		k = n / 100000;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 10000)
	{
		k = (n % 100000) / 10000;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 1000)
	{
		k = (n % 10000) / 1000;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 100)
	{
		k = (n % 1000) / 100;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 10)
	{
		k = (n % 100) / 10;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	k = n % 10;
	DrawNumber(k, x, y, scale, r, g, b);
}

int CNewHud::DrawNumberString(int index, int iX, int iY, int number, int r, int g, int b, int a, int iDrawZero, int maxsize, int widthplus)
{
	if (maxsize <= 0)
	{
		maxsize = 1;

		for (int num = 10; (number / num) > 0; num *= 10)
			maxsize++;
	}

	if (maxsize > 255)
		maxsize = 255;

	bool bShouldDraw = false;

	for (int i = 0; i < maxsize; i++)
	{
		int div = 1;
		for (int j = 0; j < maxsize - i; j++)
			div *= 10;

		int iNum = (number % div * 10) / div;

		if (iNum)
			bShouldDraw = true;

		if (!iDrawZero && !iNum && !bShouldDraw && i != maxsize - 1)
			continue;

#if 1
		int texid = 0;
		if (index == m_iNum_BigBlue)
		{
			texid = m_iNum_BigBlueC[iNum];
		}
		else if (index == m_iNum_BigRed)
		{
			texid = m_iNum_BigRedC[iNum];
		}
		else if (index == m_iNum_BigWhite)
		{
			texid = m_iNum_BigWhiteC[iNum];
		}
		else if (index == m_iNum_Bottom)
		{
			texid = m_iNum_BottomC[iNum];
		}
		else if (index == m_iNum_Center)
		{
			texid = m_iNum_CenterC[iNum];
		}
		else if (index == m_iNum_HumanScenario)
		{
			texid = m_iNum_HumanScenarioC[iNum];
		}
		else if (index == m_iNum_HumanSmall)
		{
			texid = m_iNum_HumanSmallC[iNum];
		}
		else if (index == m_iNum_SmallBlue)
		{
			texid = m_iNum_SmallBlueC[iNum];
		}
		else if (index == m_iNum_SmallRed)
		{
			texid = m_iNum_SmallRedC[iNum];
		}
		else if (index == m_iNum_Character)
		{
			texid = m_iNum_CharacterC[iNum];
		}

		if (texid)
		{
			MH_DrawTGAFunction(g_Texture[texid].iTexture, r, g, b, a, iX, iY, 1);
			iX += g_Texture[texid].iWidth + widthplus;
		}
#else
		DrawNumber(index, iNum, iX, iY, r, g, b, a);
		iX += GetNumberWidth(index) + widthplus;
#endif
	}
	return iX;
}

int CNewHud::GetNumberTotalWidth(int index, int number, int iDrawZero, int maxsize, int widthplus)
{
	if (!index)
		return 0;

	if (maxsize <= 0)
	{
		maxsize = 1;

		for (int num = 10; (number / num) > 0; num *= 10)
			maxsize++;
	}

	if (maxsize > 255)
		maxsize = 255;

	int iW = 0;
	bool bShouldDraw = false;

	for (int i = 0; i < maxsize; i++)
	{
		int div = 1;
		for (int j = 0; j < maxsize - i; j++)
			div *= 10;

		int iNum = (number % div * 10) / div;

		if (iNum)
			bShouldDraw = true;

		if (!iDrawZero && !iNum && !bShouldDraw && i != maxsize - 1)
			continue;

#if 1
		int texid = 0;
		if (index == m_iNum_BigBlue)
			texid = m_iNum_BigBlueC[iNum];
		else if (index == m_iNum_BigRed)
			texid = m_iNum_BigRedC[iNum];
		else if (index == m_iNum_BigWhite)
			texid = m_iNum_BigWhiteC[iNum];
		else if (index == m_iNum_Bottom)
			texid = m_iNum_BottomC[iNum];
		else if (index == m_iNum_Center)
			texid = m_iNum_CenterC[iNum];
		else if (index == m_iNum_HumanScenario)
			texid = m_iNum_HumanScenarioC[iNum];
		else if (index == m_iNum_HumanSmall)
			texid = m_iNum_HumanSmallC[iNum];
		else if (index == m_iNum_SmallBlue)
			texid = m_iNum_SmallBlueC[iNum];
		else if (index == m_iNum_SmallRed)
			texid = m_iNum_SmallRedC[iNum];
		else if (index == m_iNum_Character)
			texid = m_iNum_CharacterC[iNum];

		if (texid)
			iW += g_Texture[texid].iWidth + widthplus;
#else
		iW += GetNumberWidth(index) + widthplus;
#endif
	}
	return iW;
}

int CNewHud::GetNumberWidth(int index)
{
	if (!index)
		return 0;

	return g_MHTga[index].width / 10;
}

int CNewHud::GetNumberHeight(int index)
{
	if (!index)
		return 0;

	return g_MHTga[index].height;
}

void CNewHud::DrawNumber(int index, int num, int iX, int iY, int iR, int iG, int iB, int iA)
{
	if (!index)
		return;

	int width = g_MHTga[index].width / 10.0f;
	int height = g_MHTga[index].height;

	float fRect = num / 10.0f;

	Tri_Enable(GL_TEXTURE_2D);
	Tri_Enable(GL_BLEND);
	Tri_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Tri_BindTexture(GL_TEXTURE_2D, g_MHTga[index].texid);

	gEngfuncs.pTriAPI->Color4ub(iR, iG, iB, iA);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->TexCoord2f(fRect, 1);
	gEngfuncs.pTriAPI->Vertex3f(iX, iY + height, 0);

	gEngfuncs.pTriAPI->TexCoord2f(fRect + 0.1f, 1);
	gEngfuncs.pTriAPI->Vertex3f(iX + width, iY + height, 0);

	gEngfuncs.pTriAPI->TexCoord2f(fRect + 0.1f, 0);
	gEngfuncs.pTriAPI->Vertex3f(iX + width, iY, 0);

	gEngfuncs.pTriAPI->TexCoord2f(fRect, 0);
	gEngfuncs.pTriAPI->Vertex3f(iX, iY, 0);

	gEngfuncs.pTriAPI->End();
}

int CNewHud::DrawHudNumber(int type, int iX, int iY, int number, int r, int g, int b, int a, int iDrawZero, int maxsize, int widthplus)
{
	if (maxsize <= 0)
	{
		maxsize = 1;

		for (int num = 10; (number / num) > 0; num *= 10)
			maxsize++;
	}

	if (maxsize > 255)
		maxsize = 255;

	int index = !type ? m_iNumberZero : m_iDollarNumberZero;
	int width = !type ? m_iFontWidth : m_iFontWidth_Dollar;

	int color = 100 * a / 255;
	ScaleColors(r, g, b, a);

	bool bShouldDraw = false;

	for (int i = 0; i < maxsize; i++)
	{
		int div = 1;
		for (int j = 0; j < maxsize - i; j++)
			div *= 10;

		int iNum = (number % div * 10) / div;

		if (iNum)
			bShouldDraw = true;

		if (!iDrawZero && !iNum && !bShouldDraw && i != maxsize - 1)
			continue;

		if (!iNum && !bShouldDraw)
			gEngfuncs.pfnSPR_Set(GetSprite(index), color, color, color);
		else
			gEngfuncs.pfnSPR_Set(GetSprite(index + iNum), r, g, b);

		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(index + iNum));
		iX += width + widthplus;
	}
	return iX;
}

int CNewHud::GetHudNumberWidth(int type, int number, int iDrawZero, int maxsize, int widthplus)
{
	if (maxsize <= 0)
	{
		maxsize = 1;

		for (int num = 10; (number / num) > 0; num *= 10)
			maxsize++;
	}

	if (maxsize > 255)
		maxsize = 255;

	int iW = 0;
	int width = !type ? m_iFontWidth : m_iFontWidth_Dollar;

	bool bShouldDraw = false;

	for (int i = 0; i < maxsize; i++)
	{
		int div = 1;
		for (int j = 0; j < maxsize - i; j++)
			div *= 10;

		int iNum = (number % div * 10) / div;

		if (iNum)
			bShouldDraw = true;

		if (!iDrawZero && !iNum && !bShouldDraw && i != maxsize - 1)
			continue;

		iW += width + widthplus;
	}
	return iW;
}

void CNewHud::DisplayPlayerState(float flTime)
{
	if ((WeaponManager().GetHideWeapon() & (HIDEWEAPON_HEALTH | HIDEWEAPON_ALL)) || gEngfuncs.IsSpectateOnly())
		return;

	int iX = 10;
	int iY = g_sScreenInfo.iHeight - 15 - m_iFontHeight;

	int width = GetSpriteRect(m_iCross).right - GetSpriteRect(m_iCross).left;
	int height = GetSpriteRect(m_iCross).bottom - GetSpriteRect(m_iCross).top;

	int r = 255, g = 255, b = 255;
	if (g_iHealth <= 25)
		g = b = 0;

	gEngfuncs.pfnSPR_Set(GetSprite(m_iCross), r, g, b);
	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY + abs(m_iFontHeight - height) / 2, &GetSpriteRect(m_iCross));
	iX += width + 3;

	iX = DrawHudNumber(0, iX, iY, g_iHealth, r, g, b, 255, FALSE, 5);
	iX += m_iFontWidth * 3;

	if (g_iArmorType >= 0 && g_iArmorType < 2)
	{
		if (WeaponManager().GetWeaponBit() & (1 << (WEAPON_VEST)))
		{
			iY += abs(m_iFontHeight - height) / 2;
			gEngfuncs.pfnSPR_Set(GetSprite(m_iSuit[g_iArmorType]), 255, 255, 255);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(m_iSuit[g_iArmorType]));
			wrect_t rcFullArmo = *&GetSpriteRect(m_iSuitHelmet[g_iArmorType]);
			rcFullArmo.top += (rcFullArmo.bottom - rcFullArmo.top) * ((float)(100 - (min(100, g_iArmor))) * 0.01);
			if (rcFullArmo.bottom > rcFullArmo.top)
			{
				gEngfuncs.pfnSPR_Set(GetSprite(m_iSuitHelmet[g_iArmorType]), 255, 255, 255);
				gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY + rcFullArmo.top - GetSpriteRect(m_iSuitHelmet[g_iArmorType]).top, &rcFullArmo);
			}
			iX += width + 3;
			iY -= abs(m_iFontHeight - height) / 2;
			DrawHudNumber(0, iX, iY, g_iArmor, 255, 255, 255, 255, FALSE, 4);
		}
	}

	if (m_iClass)
	{
		iY = g_sScreenInfo.iHeight - 50;
		MH_DrawTGAFunction2(g_Texture[m_iClass].iTexture, 0, iY - g_Texture[m_iClass].iHeight, g_Texture[m_iClass].iWidth, g_Texture[m_iClass].iHeight, 200);
	}

	static int g_iShowPlayerState;
	g_iShowPlayerState = g_ClassData[ClassManager().GetUserClass(gEngfuncs.GetLocalPlayer()->index)].m_iBitsShowState;

	if (g_iShowPlayerState)
	{
		iX = 100;
		iY = g_sScreenInfo.iHeight - 45;

		int count = 0;
		for (int i = 0; i < 2; i++)
		{
			if (g_iShowPlayerState & (1 << i))
				count++;
		}

		if (count)
			iY -= 45 / (count + 1) * count;

		int fHeight = GetNumberHeight(m_iNum_Character);

		if (g_iShowPlayerState & (1 << SHOW_SPEED))
		{
			MH_DrawTGAFunction2(g_Texture[m_iIcon_Speed].iTexture, iX, iY - g_Texture[m_iIcon_Speed].iHeight / 2, g_Texture[m_iIcon_Speed].iWidth, g_Texture[m_iIcon_Speed].iHeight, 255);

			iX += 80;

			int iSpeed;
			iSpeed = (int)VectorLength(g_velocity);
			if (iSpeed % 10 > 5)
				iSpeed = iSpeed / 10 * 10 + 5;
			else
				iSpeed = iSpeed / 10 * 10;

			if (iSpeed > 9000)
				iSpeed = 9000;

			DrawNumberString(m_iNum_Character, iX - GetNumberTotalWidth(m_iNum_Character, iSpeed, 0, 0, 1) / 2, iY - fHeight / 2 + 3, iSpeed, 72, 156, 255, 255, 0, 0, 1);

			iY += fHeight + 3;
			iX = 100;
		}

		if (g_iShowPlayerState & (1 << SHOW_DAMAGE))
		{
			MH_DrawTGAFunction2(g_Texture[m_iIcon_Damage].iTexture, iX, iY - g_Texture[m_iIcon_Damage].iHeight / 2, g_Texture[m_iIcon_Damage].iWidth, g_Texture[m_iIcon_Damage].iHeight, 255);

			iX += 80;
			DrawNumberString(m_iNum_Character, iX - GetNumberTotalWidth(m_iNum_Character, HudShowPlayerState().m_iDamage[0], 0, 0, 1) / 2, iY - fHeight / 2 + 3, HudShowPlayerState().m_iDamage[0], 255, 187, 0, 255, 0, 0, 1);

			iX += 80;
			DrawNumberString(m_iNum_Character, iX - GetNumberTotalWidth(m_iNum_Character, HudShowPlayerState().m_iDamage[1], 0, 0, 1), iY - fHeight / 2 + 3, HudShowPlayerState().m_iDamage[1], 255, 80, 80, 255, 0, 0, 1);
		}
	}
}

void CNewHud::DisplayWeapon(float flTime)
{
	if ((WeaponManager().GetHideWeapon() & (HIDEWEAPON_WEAPON | HIDEWEAPON_ALL)) || gEngfuncs.IsSpectateOnly())
		return;
	if (!(WeaponManager().GetWeaponBit() & (1 << (WEAPON_VEST))))
		return;

	int g_iWeapon = WeaponManager().GetCurrentWeapon();

	if (g_iWeapon <= 0)
		return;
	if (g_WeaponData[g_iWeapon].iAmmoType <= 0 && !WeaponManager().GetExtraAmmoDraw())
		return;
	if (WeaponManager().GetExtraAmmoDraw() == 7)
		return;

	int weaponid = WeaponManager().GetUserWeapon(0, 0, 1);

	int r, g, b;
	int iIconWidth = g_WeaponData[g_iWeapon].rcAmmoIcon.right - g_WeaponData[g_iWeapon].rcAmmoIcon.left;
	int iIconHeight = g_WeaponData[g_iWeapon].rcAmmoIcon.bottom - g_WeaponData[g_iWeapon].rcAmmoIcon.top;
	int iInfinityWidth = GetSpriteRect(g_infinite).right - GetSpriteRect(g_infinite).left;
	int iInfinityHeight = GetSpriteRect(g_infinite).bottom - GetSpriteRect(g_infinite).top;

	int iX = g_sScreenInfo.iWidth - 5;
	int iY = g_sScreenInfo.iHeight - 15 - m_iFontHeight;

	switch (WeaponManager().GetExtraAmmoDraw())
	{
	case 2:
	case 3:
	case 4:
	case 6:
	{
		UnpackRGB(r, g, b, RGB_BRIGHTBLUE);
		break;
	}
	default:
	{
		r = g = b = 255;
		break;
	}
	}

	iX -= iIconWidth;
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	gEngfuncs.pfnSPR_Set(g_WeaponData[g_iWeapon].hAmmoIcon, r, g, b);
	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - abs(m_iFontHeight - iIconHeight) / 2, &g_WeaponData[g_iWeapon].rcAmmoIcon);

	if (g_WeaponData[g_iWeapon].iClip < 0 || (WeaponManager().GetExtraAmmoDraw() >= 2 && WeaponManager().GetExtraAmmoDraw() <= 4) || WeaponManager().GetExtraAmmoDraw() == 6 || WeaponManager().GetExtraAmmoDraw() == 8)
	{
		bool bInfinite;

		if (g_bitsInfinite & (1 << INFINITE_CLIP) && WeaponManager().GetExtraAmmoDraw() != 2)
			bInfinite = true;
		if (g_bitsInfinite & (1 << INFINITE_AMMO) && !WeaponManager().GetExtraAmmoDraw())
			bInfinite = true;

		if (bInfinite)
		{
			iX -= m_iFontWidth / 2 + iInfinityWidth;
			gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
			gEngfuncs.pfnSPR_Set(GetSprite(g_infinite), r, g, b);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(g_infinite));
		}
		else
		{
			int iAmmo = 0;
			switch (WeaponManager().GetExtraAmmoDraw())
			{
			case 3:
			case 6:
			case 8:iAmmo = g_WeaponData[g_iWeapon].iClip; break;
			case 4:iAmmo = g_WeaponData[g_iWeapon].iClip + WeaponManager().GetAmmo(); break;
			default:iAmmo = WeaponManager().GetAmmo();
			}
			iX -= m_iFontWidth / 2;
			DrawHudNumber(0, iX - GetHudNumberWidth(0, iAmmo, FALSE, 3), iY, iAmmo, r, g, b, 255, FALSE, 3);
			iX -= GetHudNumberWidth(0, iAmmo, TRUE, 3);
		}
	}
	else
	{
		if (g_bitsInfinite & (1 << INFINITE_AMMO))
		{
			iX -= GetHudNumberWidth(0, WeaponManager().GetAmmo(), TRUE, 4);
			gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
			gEngfuncs.pfnSPR_Set(GetSprite(g_infinite), r, g, b);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(g_infinite));
		}
		else
		{
			DrawHudNumber(0, iX - GetHudNumberWidth(0, WeaponManager().GetAmmo(), FALSE, 4), iY, WeaponManager().GetAmmo(), r, g, b, 255, FALSE, 4);
			iX -= GetHudNumberWidth(0, WeaponManager().GetAmmo(), TRUE, 4);
		}
		iX -= m_iFontWidth / 2;

		/*gEngfuncs.pfnSPR_Set(GetSprite(m_iDivisionline), r, g, b);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - abs(m_iFontHeight - iInfinityHeight) / 2, &GetSpriteRect(m_iDivisionline));*/
		gEngfuncs.pfnFillRGBA(iX, iY, m_iFontWidth / 10, m_iFontHeight, 255, 255, 255, 100);
		iX -= m_iFontWidth / 10 + 2;

		if (g_bitsInfinite & (1 << INFINITE_CLIP))
		{
			gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
			gEngfuncs.pfnSPR_Set(GetSprite(g_infinite), r, g, b);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX - iInfinityWidth, iY, &GetSpriteRect(g_infinite));
			iX -= GetHudNumberWidth(0, g_WeaponData[g_iWeapon].iClip, TRUE, 4);
		}
		else
		{
			DrawHudNumber(0, iX - GetHudNumberWidth(0, g_WeaponData[g_iWeapon].iClip, FALSE, 4), iY, g_WeaponData[g_iWeapon].iClip, r, g, b, 255, FALSE, 4);
			iX -= GetHudNumberWidth(0, g_WeaponData[g_iWeapon].iClip, TRUE, 4);
		}
	}

	if (g_iJanusMode == 2)
	{
		UnpackRGB(r, g, b, RGB_BRIGHTBLUE);

		iIconWidth = g_WeaponData[g_iWeapon].rcAmmoIcon2.right - g_WeaponData[g_iWeapon].rcAmmoIcon2.left;
		iIconHeight = g_WeaponData[g_iWeapon].rcAmmoIcon2.bottom - g_WeaponData[g_iWeapon].rcAmmoIcon2.top;
		iX -= iIconWidth + m_iFontWidth / 2;
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
		gEngfuncs.pfnSPR_Set(g_WeaponData[g_iWeapon].hAmmoIcon2, r, g, b);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - abs(m_iFontHeight - iIconHeight) / 2, &g_WeaponData[g_iWeapon].rcAmmoIcon2);

		iX -= iInfinityWidth;
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
		gEngfuncs.pfnSPR_Set(GetSprite(g_infinite), r, g, b);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(g_infinite));
	}
	else if (WeaponManager().GetExtraAmmoDraw() == 1 || ((WeaponManager().GetExtraAmmoDraw() == 5 || WeaponManager().GetExtraAmmoDraw() == 6) && WeaponManager().GetExtraAmmo() > 0))
	{
		UnpackRGB(r, g, b, RGB_BRIGHTBLUE);

		iIconWidth = g_WeaponData[g_iWeapon].rcAmmoIcon2.right - g_WeaponData[g_iWeapon].rcAmmoIcon2.left;
		iIconHeight = g_WeaponData[g_iWeapon].rcAmmoIcon2.bottom - g_WeaponData[g_iWeapon].rcAmmoIcon2.top;
		iX -= iIconWidth + m_iFontWidth / 2;
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
		gEngfuncs.pfnSPR_Set(g_WeaponData[g_iWeapon].hAmmoIcon2, r, g, b);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &g_WeaponData[g_iWeapon].rcAmmoIcon2);

		if (g_bitsInfinite & (1 << INFINITE_EXTRAAMMO))
		{
			iX -= iInfinityWidth;
			gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
			gEngfuncs.pfnSPR_Set(GetSprite(g_infinite), r, g, b);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(g_infinite));
		}
		else
		{
			iX -= GetHudNumberWidth(0, WeaponManager().GetExtraAmmo(), FALSE, 4);
			DrawHudNumber(0, iX, iY, WeaponManager().GetExtraAmmo(), r, g, b, 255, FALSE, 4);
		}
	}
}

void CNewHud::DisplayMoney(float flTime)
{
	if (gEngfuncs.IsSpectateOnly() || !g_bAlive)
		return;

	if (g_iMod != MOD_NONE && g_iMod != MOD_ZBS && g_iMod != MOD_GHOST && g_iMod != MOD_ZBZ)
		return;

	int iX = 4, iY, iLength, iHeight;
	gEngfuncs.pfnDrawSetTextColor(0.78, 0.78, 0.78);
	gEngfuncs.pfnDrawConsoleStringLen(g_szLocation, &iLength, &iHeight);
	gEngfuncs.pfnDrawConsoleString(5, abs(g_Texture[m_iMapTitleBG].iHeight - iHeight) / 2, g_szLocation);
	iY = max(g_Texture[m_iMapTitleBG].iHeight, iHeight) + g_pOverview->m_pRect->value;

	MH_DrawTGAFunction2(g_Texture[m_iDollarBG].iTexture, 0, iY, g_Texture[m_iDollarBG].iWidth, g_Texture[m_iDollarBG].iHeight, 255);

	if (g_bInBuyzone)
	{
		int height2 = GetSpriteRect(m_iBuyzone).bottom - GetSpriteRect(m_iBuyzone).top;

		gEngfuncs.pfnSPR_Set(GetSprite(m_iBuyzone), 255, 255, 255);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY + (g_Texture[m_iDollarBG].iHeight - height2) / 2, &GetSpriteRect(m_iBuyzone));
	}

	iX = 40;

	int heightplus = 0;
	if (g_Texture[m_iDollarBG].iHeight > m_iFontHeight_Dollar)
		heightplus = (g_Texture[m_iDollarBG].iHeight - m_iFontHeight_Dollar) / 2;

	int r = 255, g = 255, b = 255;
	if (m_iMoneyChange)
	{
		float percent = 1.0f;
		if (m_flMoney > flTime)
			percent = (m_flMoney - flTime) / 3.0f;
		else
		{
			percent = 0.0f;
			m_iMoneyChange = 0;
		}

		if (m_iMoneyChange < 0)
		{
			g -= (int)(255.0f * percent);
			b -= (int)(255.0f * percent);
		}
		else
		{
			r -= (int)(100.0f * percent);
			g -= (int)(15.0f * percent);
			b -= (int)(255.0f * percent);
		}

		int a = (int)(510.0f * percent);
		if (a > 255)
			a = 255;
		else if (a < 150)
			a = 150;
		ScaleColors(r, g, b, a);
	}

	int width = GetSpriteRect(m_iDollar).right - GetSpriteRect(m_iDollar).left;
	int height = GetSpriteRect(m_iDollar).bottom - GetSpriteRect(m_iDollar).top;

	gEngfuncs.pfnSPR_Set(GetSprite(m_iDollar), r, g, b);
	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY + abs(g_Texture[m_iDollarBG].iHeight - height) / 2, &GetSpriteRect(m_iDollar));
	iX += width + m_iFontWidth_Dollar * 5 - GetHudNumberWidth(1, m_iMoney, FALSE, 5);

	DrawHudNumber(1, iX, iY + heightplus, m_iMoney, r, g, b, 255, FALSE, 5);

	if (m_iMoneyChange)
	{
		iX = 40;
		iY += max(g_Texture[m_iDollarBG].iHeight, height);

		int alpha = 255 * (m_flMoney - flTime) / 3.0f;

		if (m_iMoneyChange < 0)
		{
			r = 255;
			g = 0;
			b = 0;
		}
		else
		{
			r = 150;
			g = 240;
			b = 0;
		}
		float percent = (float)alpha / 255.0f;

		int index = m_iMoneyChange < 0 ? m_iMinus : m_iPlus;
		int width2 = GetSpriteRect(index).right - GetSpriteRect(index).left;
		gEngfuncs.pfnSPR_Set(GetSprite(index), r * percent, g * percent, b * percent);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX + abs(width - width2) / 2, iY + abs(g_Texture[m_iDollarBG].iHeight - height) / 2, &GetSpriteRect(index));
		iX += width + m_iFontWidth_Dollar * 5 - GetHudNumberWidth(1, abs(m_iMoneyChange), FALSE, 5);

		DrawHudNumber(1, iX, iY + heightplus, abs(m_iMoneyChange), r, g, b, alpha, FALSE, 5);
	}
}

void CNewHud::DisplayWeaponState(float flTime)
{
	if (gEngfuncs.IsSpectateOnly() || !g_bAlive)
		return;

	int iLocal = gEngfuncs.GetLocalPlayer()->index;
	int iX = g_sScreenInfo.iWidth - g_Texture[m_iWeaponBG].iWidth + 85, iY = g_sScreenInfo.iHeight - 45;

	for (int i = 0; i < 4; i++)
	{
		iY -= 24;
		if (!m_AmmoPickup[i].iSprite)
			continue;
		else if (m_AmmoPickup[i].flDisplayTime <= flTime)
		{
			m_AmmoPickup[i].iSprite = NULL;
			memset(&m_AmmoPickup[i].wrect, NULL, sizeof(m_AmmoPickup[i].wrect));
			m_AmmoPickup[i].iAmount = NULL;
			m_AmmoPickup[i].flDisplayTime = NULL;
			continue;
		}

		int width = m_AmmoPickup[i].wrect.right - m_AmmoPickup[i].wrect.left;
		int height = m_AmmoPickup[i].wrect.bottom - m_AmmoPickup[i].wrect.top;

		int iX2 = iX - width;
		int iY2 = iY + 12;

		float percent = 1.0f;
		if (m_AmmoPickup[i].flDisplayTime - flTime <= 2.0)
			percent = (m_AmmoPickup[i].flDisplayTime - flTime) / 2.0f;

		int alpha = (int)(255.0f * percent);

		gEngfuncs.pfnSPR_Set(m_AmmoPickup[i].iSprite, alpha, alpha, alpha);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX2, iY, &m_AmmoPickup[i].wrect);

		iX2 -= GetHudNumberWidth(1, m_AmmoPickup[i].iAmount, FALSE, 4, 3);
		DrawHudNumber(1, iX2, iY2 - m_iFontHeight_Dollar / 2, m_AmmoPickup[i].iAmount, 255, 255, 255, alpha, FALSE, 4, 3);

		int width2 = GetSpriteRect(m_iPlus).right - GetSpriteRect(m_iPlus).left;
		int height2 = GetSpriteRect(m_iPlus).bottom - GetSpriteRect(m_iPlus).top;

		gEngfuncs.pfnSPR_Set(GetSprite(m_iPlus), alpha, alpha, alpha);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX2 - width2, iY2 - height2 / 2, &GetSpriteRect(m_iPlus));
	}

	iX = g_sScreenInfo.iWidth - 3;
	iY = g_sScreenInfo.iHeight - g_Texture[m_iWeaponBG].iHeight - 5;

	int heighttext = 0;
	if (g_szCurWeapon2)
	{
		int temp;
		gEngfuncs.pfnDrawConsoleStringLen("Hello World", &temp, &heighttext);
	}

	int weapon = WeaponManager().GetCurrentWeapon();
	weapon = weapon ? weapon : g_iCurrentWeapon;
	if (weapon)
	{
		int index = 0;
		int slot = WEAPON_SLOT[weapon];
		wrect_t* rect;
		int r, g, b;
		r = g = b = 255;

		if ((g_WeaponData[weapon].iClip <= 0 && WeaponManager().GetAmmo() <= 0) || weapon == WEAPON_KNIFE)
		{
			if (weapon != WEAPON_KNIFE)
				g = b = 0;

			index = g_WeaponData[weapon].hWeapon;
			rect = &g_WeaponData[weapon].rcWeapon;

			if (vPlayer[iLocal].team == 2 && IS_ZOMBIE_MODE)
			{
				if (weapon == WEAPON_KNIFE && m_iZombieKnife_ia)
				{
					index = GetSprite(m_iZombieKnife_ia);
					rect = &GetSpriteRect(m_iZombieKnife_ia);
				}
			}
		}
		else
		{
			index = g_WeaponData[weapon].hWeaponS;
			rect = &g_WeaponData[weapon].rcWeaponS;
		}

		int width = rect->right - rect->left;
		int height = rect->bottom - rect->top;
		iY -= height + 3;

		gEngfuncs.pfnSPR_Set(index, r, g, b);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX - width, iY, rect);

		int drawExAmmo = WeaponManager().GetExtraAmmoDraw();
		if ((weapon != WEAPON_KNIFE || drawExAmmo) && drawExAmmo != 7)
		{
			int weapon2 = WeaponManager().GetUserWeapon(0, 0, TRUE);
			int width2 = 0;

			if (drawExAmmo != 3 && drawExAmmo != 8)
			{
				float percent = WeaponManager().GetAmmo();
				if (weapon2)
					percent /= g_Weapon[weapon2].iAmmo;
				else
					percent /= WEAPON_BPAMMO[weapon];

				if (percent > 1.0)
					percent = 1.0;

				if (percent > 0.0)
				{
					width2 = (int)(20.0f * percent);
					gEngfuncs.pfnFillRGBA(iX - width + 5, iY + 4, width2, 4, 0, 255, 0, 255);
				}
			}

			if (width2 < 20)
				gEngfuncs.pfnFillRGBA(iX - width + width2 + 5, iY + 4, 20 - width2, 4, 255, 255, 255, 255);

			if (drawExAmmo && drawExAmmo != 3 && drawExAmmo != 8)
				gEngfuncs.pfnFillRGBA(iX - width + 30, iY + 4, 20, 4, 255, 255, 255, 255);
		}

		iX -= g_Texture[m_iList_NewWpn].iWidth;
		iY -= g_Texture[m_iList_NewWpn].iHeight;
		MH_DrawTGAFunction2(g_Texture[m_iList_NewWpn].iTexture, iX, iY, g_Texture[m_iList_NewWpn].iWidth, g_Texture[m_iList_NewWpn].iHeight, 255);

		char temp2[128], temp[64];
		sprintf_s(temp2, "%d %s", slot, UnicodeToUTF8(g_szCurWeapon2));
		strncpy(temp, temp2, sizeof(temp));

		gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
		gEngfuncs.pfnDrawConsoleString(iX + 5, iY + abs(g_Texture[m_iList_NewWpn].iHeight - heighttext) / 2, temp);
	}

	int height3 = GetSpriteRect(m_iWeaponGet).bottom - GetSpriteRect(m_iWeaponGet).top;
	int heightadd2 = max(heighttext, height3);

	for (int i = 0; i < 4; i++)
	{
		iY -= heightadd2 + 2;
		if (!m_WeaponPickup[i].iWeapon)
			continue;
		if (m_WeaponPickup[i].flDisplayTime <= flTime)
		{
			memset(&m_WeaponPickup[i], 0, sizeof(m_WeaponPickup[i]));
			continue;
		}

		float percent = 1.0f;
		if (m_WeaponPickup[i].flDisplayTime - flTime <= 2.0)
			percent = (m_WeaponPickup[i].flDisplayTime - flTime) / 2.0f;
		int alpha = (int)(255.0f * percent);

		int weaponid = m_WeaponPickup[i].iWeapon;
		if (weaponid == WEAPON_KNIFE || g_WeaponData[weaponid].iClip > 0 || WeaponManager().GetAmmo(weaponid) > 0)
			gEngfuncs.pfnSPR_Set(GetSprite(m_iWeaponGet), alpha, alpha, alpha);
		else
			gEngfuncs.pfnSPR_Set(GetSprite(m_iWeaponGet), alpha, 0, 0);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &GetSpriteRect(m_iWeaponGet));

		gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
		gEngfuncs.pfnDrawConsoleString(iX + 5, iY + (heightadd2 - heighttext) / 2, m_WeaponPickup[i].szWeapon);
	}
}

void CNewHud::DisplayItemState(float flTime)
{
	int iX = g_sScreenInfo.iWidth - 25;
	int iY = g_sScreenInfo.iHeight - 40 - 20;

	if (m_bUseFlash)
	{
		int width = GetSpriteRect(m_iIcon_Flash[0]).right - GetSpriteRect(m_iIcon_Flash[0]).left;
		int height = GetSpriteRect(m_iIcon_Flash[0]).bottom - GetSpriteRect(m_iIcon_Flash[0]).top;

		float percent = (float)g_iFlashBat / 100.0f;

		int r, g, b;

		if (g_iFlashBat <= 20)
		{
			r = 255;
			g = b = 0;
		}
		else
			r = g = b = 255;

		iX -= width;
		int iLen = (int)((float)width * (1.0f - percent));

		static wrect_t* TempRect;
		if (iLen)
		{
			TempRect = &GetSpriteRect(m_iIcon_Flash[1]);

			wrect_t ModifyRect;
			ModifyRect.bottom = TempRect->bottom;
			ModifyRect.left = TempRect->left;
			ModifyRect.top = TempRect->top;
			ModifyRect.right = iLen + ModifyRect.left;

			gEngfuncs.pfnSPR_Set(GetSprite(m_iIcon_Flash[1]), r * 0.75, g * 0.75, b * 0.75);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - height / 2, &ModifyRect);
		}

		if (iLen < width)
		{
			TempRect = &GetSpriteRect(m_iIcon_Flash[0]);

			wrect_t ModifyRect;
			ModifyRect.bottom = TempRect->bottom;
			ModifyRect.right = TempRect->right;
			ModifyRect.top = TempRect->top;
			ModifyRect.left = ModifyRect.right - width + iLen;

			gEngfuncs.pfnSPR_Set(GetSprite(m_iIcon_Flash[0]), r, g, b);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX + iLen, iY - height / 2, &ModifyRect);
		}

		if (g_bFlashActive)
		{
			height = GetSpriteRect(m_iIcon_Flash[2]).bottom - GetSpriteRect(m_iIcon_Flash[2]).top;
			gEngfuncs.pfnSPR_Set(GetSprite(m_iIcon_Flash[2]), r, g, b);
			gEngfuncs.pfnSPR_DrawAdditive(0, g_sScreenInfo.iWidth - 25, iY - height / 2, &GetSpriteRect(m_iIcon_Flash[2]));
		}

		iX -= 10;
	}

	if (m_bHasNvg)
	{
		int index = 0;
		if (IS_ZOMBIE_MODE && vPlayer[gEngfuncs.GetLocalPlayer()->index].team == 2) // zombie
			index = m_bUsingNvg ? m_iNvgZBOn : m_iNvgZBOff;
		else
			index = m_bUsingNvg ? m_iNvgOn : m_iNvgOff;

		if (index)
		{
			int width = GetSpriteRect(index).right - GetSpriteRect(index).left;
			int height = GetSpriteRect(index).bottom - GetSpriteRect(index).top;

			iX -= width;

			gEngfuncs.pfnSPR_Set(GetSprite(index), 255, 255, 255);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - height / 2, &GetSpriteRect(index));

			iX -= 10;
		}
	}

	if (m_bHasDefuser)
	{
		if (m_iDefuser)
		{
			int width = GetSpriteRect(m_iDefuser).right - GetSpriteRect(m_iDefuser).left;
			int height = GetSpriteRect(m_iDefuser).bottom - GetSpriteRect(m_iDefuser).top;

			iX -= width;

			gEngfuncs.pfnSPR_Set(GetSprite(m_iDefuser), 255, 255, 255);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - height / 2, &GetSpriteRect(m_iDefuser));

			iX -= 10;
		}
	}

	if (m_bHasC4)
	{
		int index = m_bCanC4 ? m_iC4On : m_iC4Off;
		if (index)
		{
			int width = GetSpriteRect(index).right - GetSpriteRect(index).left;
			int height = GetSpriteRect(index).bottom - GetSpriteRect(index).top;

			iX -= width;

			gEngfuncs.pfnSPR_Set(GetSprite(index), 255, 255, 255);
			gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - height / 2, &GetSpriteRect(index));

			iX -= 10;
		}
	}
}

void CNewHud::DisplayScoreboard(float flTime)
{
	int iX = 0, iY = 0;

	int player[2];
	memset(player, 0, sizeof(player));
	int iLocal = gEngfuncs.GetLocalPlayer()->index;

	for (int i = 1; i < 33; i++)
	{
		if (!vPlayer[i].team || vPlayer[i].team == 3)
			continue;
		if (!vPlayer[i].alive)
			continue;

		player[2 - vPlayer[i].team]++;
	}

	if (gHUD.m_iModRunning == MOD_NONE)
	{
		int iW, iH;
		iW = m_iScoreboardBG->w();
		iH = m_iScoreboardBG->h();

		iX = (ScreenWidth - iW) / 2;

		m_iScoreboardBG->Draw2DQuadScaled(iX, iY, iX + iW, iY + iH);

		iY += 10;
		iX += 105;
		iW = m_iIcon_TR_Right->w();
		iH = m_iIcon_TR_Right->h();

		m_iIcon_TR_Right->Draw2DQuadScaled(iX - 5 - iW, iY, iX + iW, iY + iH);

		iX -= 15 + GetNumberTotalWidth(m_iNum_BigRed, g_SBTopLeft, TRUE, 2, 2);
		iX = DrawNumberString(m_iNum_BigRed, iX, iY + g_Texture[m_iIcon_CT_Left].iHeight, g_SBTopLeft, 255, 255, 255, 255, TRUE, 2, 2) + 90;

		MH_DrawTGAFunction2(g_Texture[m_iIcon_CT_Left].iTexture, iX + 5, iY, g_Texture[m_iIcon_CT_Left].iWidth, g_Texture[m_iIcon_CT_Left].iHeight, 255);
		DrawNumberString(m_iNum_BigBlue, iX + 15, iY + g_Texture[m_iIcon_CT_Left].iHeight, g_SBTopRight, 255, 255, 255, 255, TRUE, 2, 2);
		iY += g_Texture[m_iIcon_CT_Left].iHeight;
		iX += g_Texture[m_iScoreboardBG].iWidth / 2 - 180;

		MH_DrawTGAFunction2(g_Texture[m_iIcon_Win_Center].iTexture, iX - g_Texture[m_iIcon_Win_Center].iWidth / 2, iY, g_Texture[m_iIcon_Win_Center].iWidth, g_Texture[m_iIcon_Win_Center].iHeight, 255);
		DrawNumberString(m_iNum_Center, iX - GetNumberTotalWidth(m_iNum_Center, g_SBTopCenter, TRUE, 2, 2) / 2, iY + g_Texture[m_iIcon_Win_Center].iHeight, g_SBTopCenter, 255, 255, 255, 255, 2, 2, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 85 - GetNumberTotalWidth(m_iNum_SmallRed, player[0], TRUE, 2, 2);
		iY = 60;
		iX = DrawNumberString(m_iNum_SmallRed, iX, iY, player[0], 255, 255, 255, 255, TRUE, 2, 2);

		int timeleft = max((int)(m_flTime - flTime), 0);
		int min = timeleft / 60, sec = timeleft % 60;
		iX += 55 - g_Texture[m_iColon_Bottom].iWidth / 2 - 1;
		MH_DrawTGAFunction2(g_Texture[m_iColon_Bottom].iTexture, iX, iY, g_Texture[m_iColon_Bottom].iWidth, g_Texture[m_iColon_Bottom].iHeight, 255);
		DrawNumberString(m_iNum_Bottom, iX - GetNumberTotalWidth(m_iNum_Bottom, min, TRUE, 2, 2), iY, min, 255, 255, 255, 255, TRUE, 2, 2);
		iX += g_Texture[m_iColon_Bottom].iWidth + 2;
		DrawNumberString(m_iNum_Bottom, iX, iY, sec, 255, 255, 255, 255, TRUE, 2, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 195;
		DrawNumberString(m_iNum_SmallBlue, iX, iY, player[1], 255, 255, 255, 255, TRUE, 2, 2);
	}
	else if (g_iMod == MOD_TDM)
	{
		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2;
		MH_DrawTGAFunction2(g_Texture[m_iScoreboardBG].iTexture, iX, iY, g_Texture[m_iScoreboardBG].iWidth, g_Texture[m_iScoreboardBG].iHeight, 255);
		iY += 10;
		iX += 103;

		g_SBTopLeft = 0;
		g_SBTopRight = 0;
		for (int i = 0; i < 33; i++)
		{
			if (!vPlayer[i].team || vPlayer[i].team == 3)
				continue;

			hud_player_info_t hPlayer;
			gEngfuncs.pfnGetPlayerInfo(i, &hPlayer);

			if (!hPlayer.name)
				continue;
			if (g_plrFrag[i] <= 0)
				continue;

			if (vPlayer[i].team == 2)
				g_SBTopLeft += g_plrFrag[i];
			else
				g_SBTopRight += g_plrFrag[i];
		}

		char num[8];
		sprintf_s(num, "%02d", g_SBTopLeft);
		int max = strlen(num);

		MH_DrawTGAFunction2(g_Texture[m_iIcon_TR_Right].iTexture, iX - 5 - g_Texture[m_iIcon_TR_Right].iWidth, iY, g_Texture[m_iIcon_TR_Right].iWidth, g_Texture[m_iIcon_TR_Right].iHeight, 255);
		iX -= 33 + GetNumberTotalWidth(m_iNum_BigRed, g_SBTopLeft, TRUE, max, 2) / 2;
		DrawNumberString(m_iNum_BigRed, iX, iY + g_Texture[m_iIcon_CT_Left].iHeight + 3, g_SBTopLeft, 255, 255, 255, 255, TRUE, max, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 177;

		sprintf_s(num, "%02d", g_SBTopRight);
		max = strlen(num);

		MH_DrawTGAFunction2(g_Texture[m_iIcon_CT_Left].iTexture, iX + 5, iY, g_Texture[m_iIcon_CT_Left].iWidth, g_Texture[m_iIcon_CT_Left].iHeight, 255);

		iY += g_Texture[m_iIcon_CT_Left].iHeight;
		DrawNumberString(m_iNum_BigBlue, iX + 33 - GetNumberTotalWidth(m_iNum_BigBlue, g_SBTopRight, TRUE, max, 2) / 2, iY + 3, g_SBTopRight, 255, 255, 255, 255, TRUE, max, 2);
		iX -= 33;

		max = (g_SBTopCenter >= 100) ? 3 : 2;
		MH_DrawTGAFunction2(g_Texture[m_iIcon_TeamKill].iTexture, iX - g_Texture[m_iIcon_TeamKill].iWidth / 2, iY, g_Texture[m_iIcon_TeamKill].iWidth, g_Texture[m_iIcon_TeamKill].iHeight, 255);
		DrawNumberString(m_iNum_Center, iX - GetNumberTotalWidth(m_iNum_Center, g_SBTopCenter, TRUE, max, 2) / 2, iY + g_Texture[m_iIcon_TeamKill].iHeight + 3, g_SBTopCenter, 255, 255, 255, 255, TRUE, max, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 80 - GetNumberTotalWidth(m_iNum_SmallRed, player[0], TRUE, 2, 2);
		iY = 60;
		iX = DrawNumberString(m_iNum_SmallRed, iX, iY, player[0], 255, 255, 255, 255, TRUE, 2, 2) + 60;
		iX -= (GetNumberWidth(m_iNum_Bottom) + 2) * 3 + (g_Texture[m_iColon_Bottom].iWidth + 2);

		int timeleft = max((int)(m_flTime - flTime), 0);
		int min = timeleft / 60, sec = timeleft % 60;
		int hour = min / 60, min2 = min % 60;
		iX = DrawNumberString(m_iNum_Bottom, iX, iY, hour, 255, 255, 255, 255, TRUE, 2, 2);
		MH_DrawTGAFunction2(g_Texture[m_iColon_Bottom].iTexture, iX, iY, g_Texture[m_iColon_Bottom].iWidth, g_Texture[m_iColon_Bottom].iHeight, 255);
		iX += g_Texture[m_iColon_Bottom].iWidth + 2;
		iX = DrawNumberString(m_iNum_Bottom, iX, iY, min2, 255, 255, 255, 255, TRUE, 2, 2);
		MH_DrawTGAFunction2(g_Texture[m_iColon_Bottom].iTexture, iX, iY, g_Texture[m_iColon_Bottom].iWidth, g_Texture[m_iColon_Bottom].iHeight, 255);
		iX += g_Texture[m_iColon_Bottom].iWidth + 2;
		DrawNumberString(m_iNum_Bottom, iX, iY, sec, 255, 255, 255, 255, TRUE, 2, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 200;
		DrawNumberString(m_iNum_SmallBlue, iX, iY, player[1], 255, 255, 255, 255, TRUE, 2, 2);
	}
	else if (g_iMod == MOD_DM)
	{
		g_SBTopLeft = 0;
		g_SBTopRight = g_PlayerExtraInfo[iLocal].frags;

		for (int i = 1; i < 33; i++)
		{
			if (!vPlayer[i].team || vPlayer[i].team == 3)
				continue;

			if (g_PlayerExtraInfo[i].frags > g_SBTopLeft)
				g_SBTopLeft = g_PlayerExtraInfo[i].frags;
		}

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG_ZBG].iWidth) / 2;
		MH_DrawTGAFunction2(g_Texture[m_iScoreboardBG_ZBG].iTexture, iX, iY, g_Texture[m_iScoreboardBG_ZBG].iWidth, g_Texture[m_iScoreboardBG_ZBG].iHeight, 255);
		iY += 10;
		iX += 100;

		int max = (g_SBTopLeft >= 100) ? 3 : 2;
		MH_DrawTGAFunction2(g_Texture[m_iIcon_1st].iTexture, iX - g_Texture[m_iIcon_1st].iWidth, iY, g_Texture[m_iIcon_1st].iWidth, g_Texture[m_iIcon_1st].iHeight, 255);
		iX -= 30;
		DrawNumberString(m_iNum_BigWhite, iX - GetNumberTotalWidth(m_iNum_BigWhite, g_SBTopLeft, TRUE, max, 2) / 2, iY + g_Texture[m_iIcon_1st].iHeight + 5, g_SBTopLeft, 255, 255, 255, 255, TRUE, max, 2);
		iX += 110;

		max = (g_SBTopRight >= 100) ? 3 : 2;
		MH_DrawTGAFunction2(g_Texture[m_iIcon_My].iTexture, iX, iY, g_Texture[m_iIcon_My].iWidth, g_Texture[m_iIcon_My].iHeight, 255);
		iX += 35 - GetNumberTotalWidth(m_iNum_BigWhite, g_SBTopRight, TRUE, max, 2) / 2;
		iY += g_Texture[m_iIcon_1st].iHeight;
		DrawNumberString(m_iNum_BigWhite, iX, iY + 5, g_SBTopRight, 255, 255, 255, 255, TRUE, max, 2);

		max = (g_SBTopCenter >= 100) ? 3 : 2;
		iX = g_sScreenInfo.iWidth / 2;
		MH_DrawTGAFunction2(g_Texture[m_iIcon_KillC].iTexture, iX - g_Texture[m_iIcon_KillC].iWidth / 2, iY, g_Texture[m_iIcon_KillC].iWidth, g_Texture[m_iIcon_KillC].iHeight, 255);
		iY += g_Texture[m_iIcon_KillC].iHeight;
		DrawNumberString(m_iNum_Center, iX - GetNumberTotalWidth(m_iNum_Center, g_SBTopCenter, TRUE, max, 2) / 2, iY, g_SBTopCenter, 255, 255, 255, 255, TRUE, max, 2);

		int timeleft = max((int)(m_flTime - flTime), 0);
		int min = timeleft / 60, sec = timeleft % 60;
		int hour = min / 60, min2 = min % 60;

		iX -= (GetNumberWidth(m_iNum_Bottom) + 2) * 3 + g_Texture[m_iColon_Bottom].iWidth + 2;
		iY = 58;
		iX = DrawNumberString(m_iNum_Bottom, iX, iY, hour, 255, 255, 255, 255, TRUE, 2, 2);
		MH_DrawTGAFunction2(g_Texture[m_iColon_Bottom].iTexture, iX, iY, g_Texture[m_iColon_Bottom].iWidth, g_Texture[m_iColon_Bottom].iHeight, 255);
		iX += g_Texture[m_iColon_Bottom].iWidth + 2;
		iX = DrawNumberString(m_iNum_Bottom, iX, iY, min2, 255, 255, 255, 255, TRUE, 2, 2);
		MH_DrawTGAFunction2(g_Texture[m_iColon_Bottom].iTexture, iX, iY, g_Texture[m_iColon_Bottom].iWidth, g_Texture[m_iColon_Bottom].iHeight, 255);
		iX += g_Texture[m_iColon_Bottom].iWidth + 2;
		iX = DrawNumberString(m_iNum_Bottom, iX, iY, sec, 255, 255, 255, 255, TRUE, 2, 2);
	}
	else if (g_iMod == MOD_ZB || g_iMod == MOD_ZE || g_iMod == MOD_ZB2 || g_iMod == MOD_ZB3 || g_iMod == MOD_ZSE || g_iMod == MOD_ZB5 || g_iMod == MOD_ZB3R || g_iMod == MOD_ZBZ)
	{
		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2;
		MH_DrawTGAFunction2(g_Texture[m_iScoreboardBG].iTexture, iX, iY, g_Texture[m_iScoreboardBG].iWidth, g_Texture[m_iScoreboardBG].iHeight, 255);
		iY += 10;
		iX += 105;

		MH_DrawTGAFunction2(g_Texture[m_iIcon_ZB_Right].iTexture, iX - 5 - g_Texture[m_iIcon_ZB_Right].iWidth, iY, g_Texture[m_iIcon_ZB_Right].iWidth, g_Texture[m_iIcon_ZB_Right].iHeight, 255);
		iX -= 15 + GetNumberTotalWidth(m_iNum_BigRed, g_SBTopLeft, TRUE, 2, 2);
		iX = DrawNumberString(m_iNum_BigRed, iX, iY + g_Texture[m_iIcon_ZB_Right].iHeight + 3, g_SBTopLeft, 255, 255, 255, 255, TRUE, 2, 2) + 90;

		MH_DrawTGAFunction2(g_Texture[m_iIcon_HM_Left].iTexture, iX + 5, iY, g_Texture[m_iIcon_HM_Left].iWidth, g_Texture[m_iIcon_HM_Left].iHeight, 255);
		DrawNumberString(m_iNum_BigBlue, iX + 15, iY + g_Texture[m_iIcon_HM_Left].iHeight + 3, g_SBTopRight, 255, 255, 255, 255, TRUE, 2, 2);
		iY += g_Texture[m_iIcon_HM_Left].iHeight;
		iX += g_Texture[m_iScoreboardBG].iWidth / 2 - 180;

		iX = g_sScreenInfo.iWidth / 2;
		MH_DrawTGAFunction2(g_Texture[m_iIcon_Round].iTexture, iX - g_Texture[m_iIcon_Round].iWidth / 2, iY, g_Texture[m_iIcon_Round].iWidth, g_Texture[m_iIcon_Round].iHeight, 255);
		iY += g_Texture[m_iIcon_Round].iHeight + 3;
		MH_DrawTGAFunction2(g_Texture[m_iSlash_Center].iTexture, iX - g_Texture[m_iSlash_Center].iWidth / 2, iY, g_Texture[m_iSlash_Center].iWidth, g_Texture[m_iSlash_Center].iHeight, 255);

		DrawNumberString(m_iNum_Center, iX - g_Texture[m_iSlash_Center].iWidth / 2 - GetNumberTotalWidth(m_iNum_Center, g_SBTopCenter, TRUE, 2, 2), iY, g_SBTopCenter, 255, 255, 255, 255, TRUE, 2, 2);
		DrawNumberString(m_iNum_Center, iX + g_Texture[m_iSlash_Center].iWidth / 2, iY, g_SBTopCenter + 1, 255, 255, 255, 255, TRUE, 2, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 85 - GetNumberTotalWidth(m_iNum_SmallRed, player[0], TRUE, 2, 2);
		iY = 60;
		iX = DrawNumberString(m_iNum_SmallRed, iX, iY, player[0], 255, 255, 255, 255, TRUE, 2, 2);

		int timeleft = max((int)(m_flTime - flTime), 0);
		int min = timeleft / 60, sec = timeleft % 60;
		iX += 55 - g_Texture[m_iColon_Bottom].iWidth / 2 - 1;
		MH_DrawTGAFunction2(g_Texture[m_iColon_Bottom].iTexture, iX, iY, g_Texture[m_iColon_Bottom].iWidth, g_Texture[m_iColon_Bottom].iHeight, 255);
		DrawNumberString(m_iNum_Bottom, iX - GetNumberTotalWidth(m_iNum_Bottom, min, TRUE, 2, 2), iY, min, 255, 255, 255, 255, TRUE, 2, 2);
		iX += g_Texture[m_iColon_Bottom].iWidth + 2;
		DrawNumberString(m_iNum_Bottom, iX, iY, sec, 255, 255, 255, 255, TRUE, 2, 2);

		iX = (g_sScreenInfo.iWidth - g_Texture[m_iScoreboardBG].iWidth) / 2 + 195;
		DrawNumberString(m_iNum_SmallBlue, iX, iY, player[1], 255, 255, 255, 255, TRUE, 2, 2);
	}
}

void CNewHud::DisplayZombieTimer(float flTime)
{
	if (g_flForceRespawn_Time >= flTime)
	{
		int iX = g_sScreenInfo.iWidth - g_MHTga[m_iIcon_ZBNotDead].width - 5;
		int iY = 0;

		int iMaxNotice = 4;

		if (g_iMod == MOD_ZBU)
			iMaxNotice = 8;

		iY = 22 * iMaxNotice + 7;

		Tri_Enable(GL_TEXTURE_2D);
		Tri_Enable(GL_BLEND);
		Tri_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Tri_BindTexture(GL_TEXTURE_2D, g_MHTga[m_iIcon_ZBNotDead].texid);
		gEngfuncs.pTriAPI->Color4ub(100, 100, 100, 255);
		gEngfuncs.pTriAPI->Begin(TRI_QUADS);

		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(iX, iY + g_MHTga[m_iIcon_ZBNotDead].height, 0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f(iX + g_MHTga[m_iIcon_ZBNotDead].width, iY + g_MHTga[m_iIcon_ZBNotDead].height, 0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f(iX + g_MHTga[m_iIcon_ZBNotDead].width, iY + g_MHTga[m_iIcon_ZBNotDead].height, 0);

		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(iX, iY + g_MHTga[m_iIcon_ZBNotDead].height, 0);
		gEngfuncs.pTriAPI->End();

		float fPercent = (g_flForceRespawn_Time - flTime) / (float)g_iForceRespawn_Max;

		if (fPercent < 1.0)
		{
			float display = 1.0f - fPercent;

			Tri_BindTexture(GL_TEXTURE_2D, g_MHTga[m_iIcon_ZBNotDead].texid);
			gEngfuncs.pTriAPI->Color4ub(255, 187, 0, 255);
			gEngfuncs.pTriAPI->Begin(TRI_QUADS);

			gEngfuncs.pTriAPI->TexCoord2f(0, 1);
			gEngfuncs.pTriAPI->Vertex3f(iX, iY + g_MHTga[m_iIcon_ZBNotDead].height, 0);

			gEngfuncs.pTriAPI->TexCoord2f(1, 1);
			gEngfuncs.pTriAPI->Vertex3f(iX + g_MHTga[m_iIcon_ZBNotDead].width, iY + g_MHTga[m_iIcon_ZBNotDead].height, 0);

			gEngfuncs.pTriAPI->TexCoord2f(1, display);
			gEngfuncs.pTriAPI->Vertex3f(iX + g_MHTga[m_iIcon_ZBNotDead].width, iY + g_MHTga[m_iIcon_ZBNotDead].height * display, 0);

			gEngfuncs.pTriAPI->TexCoord2f(0, display);
			gEngfuncs.pTriAPI->Vertex3f(iX, iY + g_MHTga[m_iIcon_ZBNotDead].height * display, 0);
			gEngfuncs.pTriAPI->End();
		}

		if (g_flForceRespawn_Time - flTime > 0.0)
		{
			if (!m_flZBIconFlash || m_flZBIconFlash <= flTime)
				m_flZBIconFlash = flTime + 1.0f;

			int alpha = 0;

			if (m_flZBIconFlash >= flTime + 0.5f)
				alpha = (int)(255.0f * (1.0f - (m_flZBIconFlash - flTime - 0.5f) / 0.5f));
			else
				alpha = (int)(255.0f * (m_flZBIconFlash - flTime) / 0.5f);

			if (alpha > 0)
			{
				float display = 1.0f - fPercent;

				Tri_BindTexture(GL_TEXTURE_2D, g_MHTga[m_iIcon_ZBNotDead].texid);
				gEngfuncs.pTriAPI->Color4ub(255, 187, 0, alpha);
				gEngfuncs.pTriAPI->Begin(TRI_QUADS);

				gEngfuncs.pTriAPI->TexCoord2f(0, display);
				gEngfuncs.pTriAPI->Vertex3f(iX, iY + g_MHTga[m_iIcon_ZBNotDead].height * display, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1, display);
				gEngfuncs.pTriAPI->Vertex3f(iX + g_MHTga[m_iIcon_ZBNotDead].width, iY + g_MHTga[m_iIcon_ZBNotDead].height * display, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(iX + g_MHTga[m_iIcon_ZBNotDead].width, iY, 0);

				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(iX, iY, 0);
				gEngfuncs.pTriAPI->End();
			}
		}

		iX += g_MHTga[m_iIcon_ZBNotDead].width / 2;
		iY += g_MHTga[m_iIcon_ZBNotDead].height + 15;

		char sz[16];
		wchar_t m_wsTime[16];
		sprintf_s(sz, "%d", (int)(g_flForceRespawn_Time - flTime));
		g_FontBold.C2W(m_wsTime, 15, sz);
		g_FontBold.SetColor(255, 187, 0, 255);
		g_FontBold.SetWidth(16);
		g_FontBold.DrawString(m_wsTime, iX - g_FontBold.GetLen(m_wsTime) / 2, iY, 100, 100);
	}
}

void CNewHud::PushAmmoPickUp(int ammoid, int amount, int ammo2)
{
	if (amount <= 0)
		return;

	int i;
	for (i = 0; i < 4; i++)
	{
		if (!m_AmmoPickup[i].iSprite)
			break;
	}
	if (i == 4)
	{
		memmove(m_AmmoPickup + 1, m_AmmoPickup, sizeof(AmmoPickup_t) * 4);
		i = 0;
	}

	for (int j = 1; j < 5; j++)
	{
		int weaponid = WeaponManager().GetUserWeapon(0, j, TRUE);
		if (weaponid)
		{
			int iWeapon = g_Weapon[weaponid].iWeaponID;
			if (iWeapon)
			{
				if (g_WeaponData[iWeapon].iAmmoType == ammoid)
				{
					if (ammo2 > 0)
					{
						m_AmmoPickup[i].iSprite = g_WeaponData[iWeapon].hAmmoIcon2;
						memcpy(&m_AmmoPickup[i].wrect, &g_WeaponData[iWeapon].rcAmmoIcon2, sizeof(wrect_t));
					}
					else
					{
						m_AmmoPickup[i].iSprite = g_WeaponData[iWeapon].hAmmoIcon;
						memcpy(&m_AmmoPickup[i].wrect, &g_WeaponData[iWeapon].rcAmmoIcon, sizeof(wrect_t));
					}

					m_AmmoPickup[i].iAmount = amount;
					m_AmmoPickup[i].flDisplayTime = cl.time + 3.0f;
					break;
				}
			}
		}
	}
}

void CNewHud::PushWeaponPickUp(char* szWeapon, int iWeapon)
{
	if (!szWeapon || !iWeapon)
		return;

	int i;
	for (i = 0; i < 4; i++)
	{
		if (!m_WeaponPickup[i].iWeapon)
			break;
	}

	if (i == 4)
	{
		for (int j = 3; j > 0; j--)
			memmove(&m_WeaponPickup[j], &m_WeaponPickup[j - 1], sizeof(WeaponPickup_t));
		i = 0;
	}

	if (IS_ZOMBIE_MODE)
	{
		if (vPlayer[gEngfuncs.GetLocalPlayer()->index].team == 2)
		{
			if (strstr(szWeapon, "grenade"))
			{
				if (iWeapon == WEAPON_HEGRENADE)
					szWeapon = "zombibomb";
				else
					szWeapon = "zombibomb2";
			}
		}
	}

	strcpy(m_WeaponPickup[i].szWeapon, UnicodeToUTF8(GetWeaponLangName(CheckCSOName(g_szCsoWpnName, szWeapon))));

	m_WeaponPickup[i].iWeapon = iWeapon;
	m_WeaponPickup[i].flDisplayTime = cl.time + 3.0f;
}

void CNewHud::SetClass(char* szClass)
{
	if (!szClass)
		return;

	if (!strcmp(m_szClass, strlwr(szClass)))
		return;

	strcpy(m_szClass, szClass);

	char szClassCSO[32];
	if (strstr(szClass, "_host"))
	{
		szClass[strlen(szClass) - 5] = 0;
		strcpy(szClassCSO, CheckCSOName(g_szCsoHudClassName, szClass));

		char temp[32];
		sprintf_s(temp, "%s_host", szClassCSO);
		strcpy(szClassCSO, temp);
	}
	else if (strstr(szClass, "_origin"))
	{
		szClass[strlen(szClass) - 7] = 0;
		strcpy(szClassCSO, CheckCSOName(g_szCsoHudClassName, szClass));
	}
	else
		strcpy(szClassCSO, CheckCSOName(g_szCsoHudClassName, szClass));

	char szImage[64];
	sprintf_s(szImage, "resource\\hud\\portrait\\hud_%s", szClassCSO);
	m_iClass = FindTexture(szImage);
}