#include "extdll_menu.h"
#include "imgui.h"
#include "imgui_utils.h"
#include "Utils.h"

#include "ConnectProgress.h"
#include "OptionsDialog.h"
#include "SimpleLogin.h"

#include "mathlib.h"
#include "const.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "entity_types.h"

extern "C" qboolean CL_IsInMenu(void);

static HIMAGE g_playerPIC = 0;

static void CalcFov(ref_menu_params_t &refdef)
{
    float x = refdef.viewport[2] / tan( DEG2RAD( refdef.fov_x ) * 0.5f );
    float half_fov_y = atan( refdef.viewport[3] / x );
    refdef.fov_y = RAD2DEG( half_fov_y ) * 2;
}

static void RenderPlayerModel()
{
    auto ent = EngFuncs::GetPlayerModel();

    int w = ScreenWidth;
    int h = ScreenHeight;

    static ref_menu_params_t refdef = {};
    refdef.fov_x = 40.0f;
    refdef.fov_y = 40.0f;
    refdef.viewport[0] = 0;
    refdef.viewport[1] = 0;
    refdef.viewport[2] = 128;
    refdef.viewport[3] = 128;
    CalcFov(refdef);

    // adjust entity params
    ent->index = 0;
    ent->curstate.body = 0;
    ent->curstate.number = 1;	// IMPORTANT: always set playerindex to 1
    ent->curstate.animtime = gpGlobals->time;	// start animation
    ent->curstate.sequence = 1;
    ent->curstate.scale = 1.0f;
    ent->curstate.frame = 0.0f;
    ent->curstate.framerate = 1.0f;
    //ent->curstate.effects |= EF_FULLBRIGHT;
    ent->curstate.controller[0] = 127;
    ent->curstate.controller[1] = 127;
    ent->curstate.controller[2] = 127;
    ent->curstate.controller[3] = 127;
    ent->latched.prevcontroller[0] = 127;
    ent->latched.prevcontroller[1] = 127;
    ent->latched.prevcontroller[2] = 127;
    ent->latched.prevcontroller[3] = 127;
    ent->origin[0] = ent->curstate.origin[0] = 45.0f / tan( DEG2RAD( refdef.fov_y / 2.0f ));
    ent->origin[2] = ent->curstate.origin[2] = 2.0f;
    ent->angles[1] = ent->curstate.angles[1] = 180.0f;

    ent->player = 1; // yes, draw me as playermodel

    ent->curstate.rendermode = kRenderTransTexture;
    ent->curstate.renderamt = 255;

    //EngFuncs::CvarSetString( "model", "player" );


    EngFuncs::ClearScene();

    refdef.time = gpGlobals->time;
    refdef.frametime = gpGlobals->frametime;

    EngFuncs::CL_CreateVisibleEntity( ET_NORMAL, ent );
    EngFuncs::RenderScene( &refdef );

    EngFuncs::SetModel( ent, "models/player.mdl" );


    // draw the player model
}

static void DrawBackground()
{
    //EngFuncs::FillRGBA(0, 0, ScreenWidth, ScreenHeight, 255, 255, 255, 255);
    int w = ScreenWidth;
    int h = ScreenHeight;
    auto drawlist = ImGui::GetBackgroundDrawList();

    drawlist->AddRectFilled(ImVec2(0 * w, 0 * h), ImVec2(1 * w, 1 * h), ImColor(255, 100, 100, 255));
    drawlist->PathClear();
    drawlist->PathLineTo(ImVec2(1 * w, 0.5 * h));
    drawlist->PathBezierCurveTo(ImVec2(0.5 * w, 0.65 * h), ImVec2(0.35 * w, 0.8 * h), ImVec2(0.25 * w, 0.9 * h));
    drawlist->PathBezierCurveTo(ImVec2(0.15 * w,1.00 * h), ImVec2(0.025 * w, 1.0 * h), ImVec2(0.0 * w, 0.9 * h));
    drawlist->PathLineTo(ImVec2(0 * w, 0 * h));
    drawlist->PathLineTo(ImVec2(1 * w, 0 * h));
    drawlist->PathFillConvex(ImColor(255, 255, 255, 255));

    drawlist->PathClear();
    drawlist->PathLineTo(ImVec2(0.2 * w, 0.8 * h));
    drawlist->PathBezierCurveTo(ImVec2(0.15 * w, 0.9 * h), ImVec2(0.1 * w, 0.92 * h), ImVec2(0.0 * w, 0.88 * h));
    drawlist->PathLineTo(ImVec2(0 * w, 0.7 * h));
    drawlist->PathFillConvex(ImColor(255, 100, 100, 255));

    drawlist->PathClear();
    drawlist->PathLineTo(ImVec2(0.2 * w, 0.8 * h));
    drawlist->PathBezierCurveTo(ImVec2(0.125 * w, 0.85 * h), ImVec2(0.1 * w, 0.86 * h), ImVec2(0.0 * w, 0.82 * h));
    drawlist->PathLineTo(ImVec2(0 * w, 0.7 * h));
    drawlist->PathFillConvex(ImColor(255, 255, 255, 255));


    drawlist->PathClear();
    drawlist->PathLineTo(ImVec2(1.0 * w, 0.6 * h));
    drawlist->PathBezierCurveTo(ImVec2(0.87 * w, 0.65 * h), ImVec2(0.45 * w, 0.97 * h), ImVec2(0.35 * w, 1 * h));
    drawlist->PathLineTo(ImVec2(1 * w, 1 * h));
    drawlist->PathFillConvex(ImColor(255, 50, 50, 200));

    {
        //EngFuncs::PIC_Set( hPic, 255, 255, 255, 255 );
        //EngFuncs::PIC_DrawAdditive( w * 0.5, h * 0.3, w * 0.5, uiStatic.buttons_draw_height, &rects[BUTTON_FOCUS] );
        if(g_playerPIC)
            drawlist->AddImage((ImTextureID)g_playerPIC, ImVec2(0.45 * w, 0.2 * h), ImVec2(1.05 * w, 1 * h));
    }


    {
        int x = 0.85 * w;
        int y = 0.05 * h;
        float fontsize = w / 1920 * 64;
        drawlist->AddText(NULL, fontsize, ImVec2(x, y), ImColor(0, 0, 0, 255), "广告位招租");
    }

    {
        int x = 0.05 * w;
        int y = 0.05 * h;
        float fontsize = w / 1920 * 64;
        drawlist->AddText(NULL, fontsize, ImVec2(x, y), ImColor(255, 100, 100, 255), "柑橘");
        ImGuiUtils::AddCitrusLogo(drawlist, ImVec2(x + fontsize * 2.5, y + fontsize * 0.5), fontsize, 0);
        drawlist->AddText(NULL, fontsize, ImVec2(x + fontsize * 3.5, y), ImColor(255, 50, 50, 255), "CitruS");
    }

    {
        int x = 0.65 * w;
        int y = 0.85 * h;
        float fontsize = w / 1920 * 64;
        drawlist->AddRectFilled(ImVec2(x, y), ImVec2(x + fontsize * 15, y + fontsize * 2), ImColor(255, 255, 0, 255), fontsize / 2);
        drawlist->AddText(NULL, fontsize, ImVec2(x + fontsize / 2, y + fontsize / 2), ImColor(0, 0, 0, 255), "唯 一 指 定 Q Q 群 ： 7 0 6 7 1 1 4 2 0");
    }

    {
        const int x0 = 0.15 * w;
        const int y0 = 0.25 * h;
        const int x1 = 0.17 * w;
        const int y1 = 0.28 * h;
        static int x = x0;
        static int y = y0;

        static float x_dir = 0;
        static float y_dir = 1;

        if(x <= x0 || x >= x1 || y <= y0 || y >= y1)
        {
            x_dir = EngFuncs::RandomFloat(-1, 1);
            y_dir = EngFuncs::RandomFloat(-1, 1);
        }
        auto deltatime = ImGui::GetIO().DeltaTime;
        x += x_dir * deltatime * w / 3;
        y += y_dir * deltatime * h / 3;
        x = std::clamp(x, x0, x1);
        y = std::clamp(y, y0, y1);

        float fontsize = w / 1920 * 64;
        drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 0), ImColor(255, 150, 0, 255), "香蕉诱惑套餐");
        drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 1.5), ImColor(0, 0, 0, 255), "ze_Ricardo_Milos_dp");
        drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 3.5), ImColor(255, 150, 0, 255), "极限冲B套餐");
        drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 5), ImColor(0, 0, 0, 255), "ze_rushb_p90");
        drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 7), ImColor(255, 150, 0, 255), "火星人套餐");
        drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 8.5), ImColor(0, 0, 0, 255), "ze_deadspace_final");
        drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 10.5), ImColor(255, 150, 0, 255), "高血压套餐");
        drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 12), ImColor(0, 0, 0, 255), "ze_FFVII_Mako_Reactor_v1_1");

        drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 0.25), ImColor(255, 0, 0, 255), "280.00");
        drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 3.75), ImColor(255, 0, 0, 255), "345.00");
        drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 7.25), ImColor(255, 0, 0, 255), "575.00");
        drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 10.75), ImColor(255, 0, 0, 255), "935.00");
    }
}

int UI_VidInit(void)
{
    const char *player_images[] = {
            "gfx/vgui/urban",
            "gfx/vgui/Guerilla",
    };
    g_playerPIC = EngFuncs::PIC_Load(player_images[0]);
	return 0;
}
void UI_Init(void)
{
	ui::ConnectProgress_Init();
	ui::OptionsDialog_Init();
	ui::SimpleLogin_Init();
}
void UI_Shutdown(void)
{

}
void UI_UpdateMenu(float flTime)
{
	if (CL_IsInMenu())
	{
		bool x = false;
		if (CL_IsActive())
		{
			ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
			EngFuncs::FillRGBA(0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 50);
			x = ImGui::Begin("柑橘CitruS", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		}
		else
		{
			ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
			DrawBackground();
			x = ImGui::Begin("柑橘CitruS", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		}
		if (x)
		{
			if (!CL_IsActive())
				ImGuiUtils::CitrusLogo("cirtus logo", ImGuiUtils::GetScaledSize(ImVec2(-1, 128)), ImGuiUtils::GetScaledValue(128 * 0.75));
			if (CL_IsActive() && ImGui::Button("返回游戏", ImVec2(-1, 0)))
			{
				EngFuncs::KEY_SetDest(KEY_GAME);
			}
			if (CL_IsActive() && ImGui::Button("断开连接", ImVec2(-1, 0)))
			{
				EngFuncs::ClientCmd(false, "disconnect\n");
			}
			if (!CL_IsActive() && ImGui::Button("连接柑橘CitruS测试服务器", ImVec2(-1, 0)))
			{
				EngFuncs::ClientCmd(false, "connect z4.moemod.com:27015\n");
			}
			if (ImGui::Button("打开控制台", ImVec2(-1, 0)))
			{
				EngFuncs::KEY_SetDest(KEY_CONSOLE);
			}
			if (ImGui::Button("游戏设置", ImVec2(-1, 0)))
			{
				ui::OptionsDialog_SetVisible(true);
			}
			if (ImGui::Button("退出游戏", ImVec2(-1, 0)))
			{
				ImGui::OpenPopup("Quit?");
			}

			if (ImGui::BeginPopupModal("Quit?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("你确定要退出游戏吗？\n\n");
				ImGui::Separator();

				if (ImGui::Button("退出", ImGuiUtils::GetScaledSize(ImVec2(120, 0)))) { EngFuncs::ClientCmd(false, "quit\n"); }
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("取消", ImGuiUtils::GetScaledSize(ImVec2(120, 0)))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

			ImGui::End();
		}

		ui::ConnectProgress_OnGUI();
		ui::OptionsDialog_OnGui();
		ui::SimpleLogin_OnGui();
	}
}
void UI_KeyEvent(int key, int down)
{

}
void UI_MouseMove(int x, int y)
{

}
void UI_SetActiveMenu(int fActive)
{
	EngFuncs::KEY_ClearStates();
	if (fActive)
	{
		EngFuncs::KEY_SetDest(KEY_MENU);
	}
}
void UI_AddServerToList(netadr_t adr, const char* info)
{

}
void UI_GetCursorPos(int* pos_x, int* pos_y)
{

}
void UI_SetCursorPos(int pos_x, int pos_y)
{

}
void UI_ShowCursor(int show)
{
	
}
void UI_CharEvent(int key)
{

}
int UI_MouseInRect(void)
{
	return 0;
}
int UI_IsVisible(void)
{
	return 0;
}
int UI_CreditsActive(void)
{
	return 0;
}
void UI_FinalCredits(void)
{

}
void UI_OnGUI(struct ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
}
extern "C" EXPORT void AddTouchButtonToList(const char* name, const char* texture, const char* command, unsigned char* color, int flags)
{

}