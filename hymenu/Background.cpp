#include "extdll_menu.h"
#include "imgui.h"
#include "imgui_utils.h"
#include "Utils.h"

#include "qrcode.h"

#include "mathlib.h"
#include "const.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "entity_types.h"

#include <vector>

namespace ui {
    static HIMAGE g_playerPIC = 0;

    static QRCode s_qrcode;
    static std::vector<uint8_t> s_qrCodeBytes;

    void Background_VidInit()
    {
        const char *player_images[] = {
                "gfx/vgui/urban",
                "gfx/vgui/Guerilla",
        };
        g_playerPIC = EngFuncs::PIC_Load(player_images[0]);

        // The structure to manage the QR code
        s_qrCodeBytes.resize(qrcode_getBufferSize(9));
        qrcode_initText(&s_qrcode, s_qrCodeBytes.data(), 9, ECC_LOW, "https://qm.qq.com/cgi-bin/qm/qr?k=m-wI8acAT7XurfGiy_2JZy9DBupc1tXe&authKey=ZgiTHa6sHJjpkKqMLE8XQH31pHL1gFGF4mOi3xH5zW6D169YuTGbKJqcRI0B7cVX");
    }

    static void CalcFov(ref_menu_params_t &refdef) {
        float x = refdef.viewport[2] / tan(DEG2RAD(refdef.fov_x) * 0.5f);
        float half_fov_y = atan(refdef.viewport[3] / x);
        refdef.fov_y = RAD2DEG(half_fov_y) * 2;
    }

    static void RenderPlayerModel() {
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
        ent->curstate.number = 1;    // IMPORTANT: always set playerindex to 1
        ent->curstate.animtime = gpGlobals->time;    // start animation
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
        ent->origin[0] = ent->curstate.origin[0] = 45.0f / tan(DEG2RAD(refdef.fov_y / 2.0f));
        ent->origin[2] = ent->curstate.origin[2] = 2.0f;
        ent->angles[1] = ent->curstate.angles[1] = 180.0f;

        ent->player = 1; // yes, draw me as playermodel

        ent->curstate.rendermode = kRenderTransTexture;
        ent->curstate.renderamt = 255;

        //EngFuncs::CvarSetString( "model", "player" );


        EngFuncs::ClearScene();

        refdef.time = gpGlobals->time;
        refdef.frametime = gpGlobals->frametime;

        EngFuncs::CL_CreateVisibleEntity(ET_NORMAL, ent);
        EngFuncs::RenderScene(&refdef);

        EngFuncs::SetModel(ent, "models/player.mdl");


        // draw the player model
    }

    void Background_OnGUI() {
        //EngFuncs::FillRGBA(0, 0, ScreenWidth, ScreenHeight, 255, 255, 255, 255);
        int w = ScreenWidth;
        int h = ScreenHeight;
        auto drawlist = ImGui::GetBackgroundDrawList();

        drawlist->AddRectFilled(ImVec2(0 * w, 0 * h), ImVec2(1 * w, 1 * h), ImColor(255, 100, 100, 255));
        drawlist->PathClear();
        drawlist->PathLineTo(ImVec2(1 * w, 0.5 * h));
        drawlist->PathBezierCurveTo(ImVec2(0.5 * w, 0.65 * h), ImVec2(0.35 * w, 0.8 * h), ImVec2(0.25 * w, 0.9 * h));
        drawlist->PathBezierCurveTo(ImVec2(0.15 * w, 1.00 * h), ImVec2(0.025 * w, 1.0 * h), ImVec2(0.0 * w, 0.9 * h));
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
            if (g_playerPIC)
                drawlist->AddImage((ImTextureID) g_playerPIC, ImVec2(0.45 * w, 0.2 * h), ImVec2(1.05 * w, 1 * h));
        }


        {
            int x = 0.85 * w;
            int y = 0.05 * h;
            float fontsize = w / 1920 * 72;
            drawlist->AddText(NULL, fontsize, ImVec2(x, y), ImColor(0, 0, 0, 255), "* 搔 马 加 裙 *");
        }

        {
            int x0 = 0.825 * w;
            int y0 = 0.1 * h;
            float fontsize = w / 1920 * 10;

            for (std::uint8_t y = 0; y < s_qrcode.size; y++) {
                for (std::uint8_t x = 0; x < s_qrcode.size; x++) {
                    auto col = ImColor(255, 255, 255, 255);
                    if (qrcode_getModule(&s_qrcode, x, y)) {
                        col = ImColor(0, 0, 0, 255);
                    }
                    //drawlist->AddCircleFilled(ImVec2(x0 + x * fontsize, y0 + y * fontsize), fontsize / 2, col);
                    drawlist->AddRectFilled( ImVec2(x0 + x * fontsize, y0 + y * fontsize), ImVec2(x0 + x * fontsize + fontsize, y0 + y * fontsize + fontsize), col );
                }
            }
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
            drawlist->AddRectFilled(ImVec2(x, y), ImVec2(x + fontsize * 15, y + fontsize * 2),
                                    ImColor(255, 255, 0, 255), fontsize / 2);
            drawlist->AddText(NULL, fontsize, ImVec2(x + fontsize / 2, y + fontsize / 2), ImColor(0, 0, 0, 255),
                              "唯 一 指 定 Q Q 群 ： 7 0 6 7 1 1 4 2 0");
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

            if (x <= x0 || x >= x1 || y <= y0 || y >= y1) {
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
            drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 1.5), ImColor(0, 0, 0, 255),
                              "ze_Ricardo_Milos_dp");
            drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 3.5), ImColor(255, 150, 0, 255), "极限冲B套餐");
            drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 5), ImColor(0, 0, 0, 255), "ze_rushb_p90");
            drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 7), ImColor(255, 150, 0, 255), "火星人套餐");
            drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 8.5), ImColor(0, 0, 0, 255),
                              "ze_deadspace_final");
            drawlist->AddText(NULL, fontsize * 1.5, ImVec2(x, y + fontsize * 10.5), ImColor(255, 150, 0, 255), "高血压套餐");
            drawlist->AddText(NULL, fontsize, ImVec2(x, y + fontsize * 12), ImColor(0, 0, 0, 255),
                              "ze_FFVII_Mako_Reactor_v1_1");

            drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 0.25),
                              ImColor(255, 0, 0, 255), "280.00");
            drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 3.75),
                              ImColor(255, 0, 0, 255), "345.00");
            drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 7.25),
                              ImColor(255, 0, 0, 255), "575.00");
            drawlist->AddText(NULL, fontsize * 2, ImVec2(x + fontsize * 10, y + fontsize * 10.75),
                              ImColor(255, 0, 0, 255), "935.00");
        }
    }

}