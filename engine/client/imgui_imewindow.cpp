/*
imgui_ime_window.cpp
Copyright (C) 2020 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

extern "C" {
#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "input_ime.h"
}

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_utils.h"
#include "imgui_imewindow.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

static bool show_ime_candidate_list = true;

void ImGui_ImeWindow_Init()
{
	
}

// https://github.com/maildrop/DearImGui-with-IMM32/blob/master/IMM32IMGUI/imgui_imm32_onthespot.cpp
void ImGui_ImeWindow_OnGUI(void)
{
    ImGuiIO& io = ImGui::GetIO();

    static ImVec2 window_pos = ImVec2();
    static ImVec2 window_pos_pivot = ImVec2();

    /*
       #1 Candidate List Window の位置に関する保持
       Candidate List をクリックしたときに、ウィンドウ位置を動かさない。
       クリック後に、TextInputを復帰させる処理
       see #1
    */
    static ImGuiID candidate_window_root_id = 0;

    static ImGuiWindow* lastTextInputNavWindow = nullptr;
    static ImGuiID lastTextInputActiveId = 0;
    static ImGuiID lastTextInputFocusId = 0;

    // Candidate Window をフォーカスしている時は Window の位置を操作しない 
    if (!(candidate_window_root_id &&
        ((ImGui::GetCurrentContext()->NavWindow ? ImGui::GetCurrentContext()->NavWindow->RootWindow->ID : 0u) == candidate_window_root_id))) {

        window_pos = ImVec2(ImGui::GetCurrentContext()->PlatformImePos.x + 1.0f,
            ImGui::GetCurrentContext()->PlatformImePos.y); // 
        window_pos_pivot = ImVec2(0.0f, 0.0f);

        const ImGuiContext* const currentContext = ImGui::GetCurrentContext();
        IM_ASSERT(currentContext || !"ImGui::GetCurrentContext() return nullptr.");
        if (currentContext) {
            // mouse press してる間は、ActiveID が切り替わるので、
            if (!ImGui::IsMouseClicked(0)) {
                if ((currentContext->WantTextInputNextFrame != -1) ? (!!(currentContext->WantTextInputNextFrame)) : false) {
                    if ((!!currentContext->NavWindow) &&
                        (currentContext->NavWindow->RootWindow->ID != candidate_window_root_id) &&
                        (ImGui::GetActiveID() != lastTextInputActiveId)) {
                        lastTextInputNavWindow = ImGui::GetCurrentContext()->NavWindow;
                        lastTextInputActiveId = ImGui::GetActiveID();
                        lastTextInputFocusId = ImGui::GetFocusID();
                    }
                }
                else {
                    lastTextInputNavWindow = nullptr;
                    lastTextInputActiveId = 0;
                    lastTextInputFocusId = 0;
                }
            }
        }
    }

    ImVec2 target_screen_pos = ImGui::GetCurrentContext()->PlatformImePos;

    std::vector<const char*> candidate_list;
    auto candidate_count = IME_GetCandidateListCount();
    for (size_t i = 0; i < candidate_count; ++i)
        candidate_list.push_back(IME_GetCandidate(i));

    auto selection = IME_GetCandidateListSelectedItem();
    auto candidate_window_num = IME_GetCandidateListPageSize();

    const char* completed = IME_GetCompositionString();

    /* Draw Candidate List */
    if (candidate_count || completed[0]) {

        std::vector<const char*> listbox_items = {};
        int candidate_selection = 0;

        /* ページに分割します */
        // TODO:candidate_window_num の値に注意 0 除算の可能性がある。
        if (candidate_window_num)
        {
            int candidate_page = ((int)selection) / candidate_window_num;
            candidate_selection = ((int)selection) % candidate_window_num;

            auto begin_ite = std::begin(candidate_list);
            std::advance(begin_ite, candidate_page * candidate_window_num);
            auto end_ite = begin_ite;
            {
                auto the_end = std::end(candidate_list);
                for (int i = 0; end_ite != the_end && i < candidate_window_num; ++i) {
                    std::advance(end_ite, 1);
                }
            }

            std::copy(begin_ite, end_ite, std::back_inserter(listbox_items));
        }

        /* もし candidate window が画面の外に出ることがあるのならば、上に表示する */
        const float candidate_window_height =
            ((ImGui::GetStyle().FramePadding.y * 2) +
                ((ImGui::GetTextLineHeightWithSpacing()) * (std::min<int>(10, std::size(listbox_items)) + 3)));

        if (io.DisplaySize.y < (target_screen_pos.y + candidate_window_height)) {
            target_screen_pos.y -=
                ImGui::GetTextLineHeightWithSpacing() + candidate_window_height;
        }

        ImGui::SetNextWindowPos(target_screen_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowSize(ImVec2(0, candidate_window_height), ImGuiCond_Always);

        if (ImGui::Begin("##Overlay-IME-Candidate-List-Window",
            nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing)) {
            if (ImGui::ListBoxHeader("##IMECandidateListWindow",
                static_cast<int>(std::size(listbox_items)),
                std::min<int>(10, std::size(listbox_items)))) {

                int i = 0; // for の最後で、i をインクリメントしているので、注意 
                for (const char*& listbox_item : listbox_items) {
                    ImGui::Text("%d.", i + IME_CandidateListStartsAtOne());
                    ImGui::SameLine();
                    if (ImGui::Selectable(listbox_item, (i == candidate_selection))) {

                        /* candidate list selection */

                        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                            !ImGui::IsAnyItemActive() &&
                            !ImGui::IsMouseClicked(0)) {
                            if (lastTextInputActiveId && lastTextInputFocusId) {
                                ImGui::SetActiveID(lastTextInputActiveId, lastTextInputNavWindow);
                                ImGui::SetFocusID(lastTextInputFocusId, lastTextInputNavWindow);
                            }
                        }

                        /*
                          ImmNotifyIME (hImc, NI_SELECTCANDIDATESTR, 0, candidate_page* candidate_window_num + i)); をしたいのだが、
                          Vista 以降 ImmNotifyIME は NI_SELECTCANDIDATESTR はサポートされない。
                          @see IMM32互換性情報.doc from Microsoft

                          そこで、DXUTguiIME.cpp (かつて使われていた DXUT の gui IME 処理部 現在は、deprecated 扱いで、
                          https://github.com/microsoft/DXUT で確認出来る
                          当該のコードは、https://github.com/microsoft/DXUT/blob/master/Optional/DXUTguiIME.cpp )
                          を確認したところ

                          L.380から で Candidate List をクリックしたときのコードがある

                          どうしているかというと SendKey で、矢印カーソルキーを送ることで、Candidate Listからの選択を行っている。
                          （なんということ？！）

                          これを根拠に SendKey を利用したコードを作成する。
                        */
                        {
                            /*
                              これで、選択された変換候補が末尾の場合は確定、
                              そうでない場合は、次の変換文節を選択させたいのであるが、
                              keybd_event で状態を送っているので、PostMessage でその処理を遅らせる
                              この request_candidate_list_str_commit は、 WM_IME_COMPOSITION の最後でチェックされ
                              WM_APP + 200 を PostMessage して、そこで実際の確定動作が行われる。
                            */

                            // TODO : 上屏
                            /*
                            if (candidate_selection == i) {
                                OutputDebugStringW(L"complete\n");
                                this->request_candidate_list_str_commit = 1;
                            }
                            else {
                                const BYTE nVirtualKey = (candidate_selection < i) ? VK_DOWN : VK_UP;
                                const size_t nNumToHit = abs(candidate_selection - i);
                                for (size_t hit = 0; hit < nNumToHit; ++hit) {
                                    keybd_event(nVirtualKey, 0, 0, 0);
                                    keybd_event(nVirtualKey, 0, KEYEVENTF_KEYUP, 0);
                                }
                                this->request_candidate_list_str_commit = (int)nNumToHit;
                            }
                            */
                        }

                    }

                    if (i == candidate_selection)
                        ImGui::SetItemDefaultFocus();
                    ++i;
                }
                ImGui::ListBoxFooter();
            }
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            ImGui::TextUnformatted(completed);
            ImGui::PopStyleColor();

            // #1 ここで作るウィンドウがフォーカスを持ったときには、ウィンドウの位置を変更してはいけない。
            candidate_window_root_id = ImGui::GetCurrentWindowRead()->RootWindow->ID;
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        }
        ImGui::End();
    }
}
