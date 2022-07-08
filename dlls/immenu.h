#pragma once

#include "newmenus.h"

namespace sv {
    class ImMenuContext
    {
    public:
        virtual bool begin(const std::string& title) = 0;
        virtual void enabled() = 0;
        virtual void disabled() = 0;
        virtual bool item(const std::string& info, const std::string& text) = 0;
        virtual bool end(CBasePlayer* client, duration_t time = {}) = 0;
    protected:
        virtual ~ImMenuContext() = 0;
    };
    inline ImMenuContext::~ImMenuContext() = default;

    namespace detail {
        class ImMenuBuilder : public ImMenuContext
        {
        public:
            ImMenuBuilder(PlayerMenu* m) : menu(m), next_disabled(false) {}

            bool begin(const std::string& title) override
            {
                menu->m_Title = title.c_str();
                return true;
            }

            void enabled() override
            {
                next_disabled = false;
            }

            void disabled() override
            {
                next_disabled = true;
            }

            bool item(const std::string& info, const std::string& text) override
            {
                if(next_disabled)
                    menu->AddItem("\\d" + text, info);
                else
                    menu->AddItem(text, info);
                return false;
            }

            bool end(CBasePlayer* client, duration_t time = {}) override
            {
                menu->Display(client, 0, time);
                return false;
            }

        private:
            PlayerMenu* const menu;
            bool next_disabled;
        };

        class ImMenuCaller : public ImMenuContext
        {
        public:
            ImMenuCaller(PlayerMenu* m, item_t item)
                    : menu(m), next_disabled(false), selected_item(m->GetMenuItem(item))
            {

            }

            bool begin(const std::string& title) override
            {
                return true;
            }

            void enabled() override
            {
                next_disabled = false;
            }

            void disabled() override
            {
                next_disabled = true;
            }

            bool item(const std::string& info, const std::string& text) override
            {
                return !std::exchange(next_disabled, false) && info == selected_item->cmd;
            }

            bool end(CBasePlayer* voter, duration_t time = {}) override
            {
                return true;
            }

        private:
            PlayerMenu* const menu;
            bool next_disabled;
            const menuitem *selected_item;
        };
    }

    template<class Fn = void(*)(ImMenuContext&&)>
    auto ImMenu(CBasePlayer* pPlayer, Fn &&fn) -> typename std::enable_if<std::is_invocable<Fn, detail::ImMenuCaller, CBasePlayer *>::value&& std::is_invocable<Fn, detail::ImMenuBuilder, CBasePlayer *>::value>::type
    {
        PlayerMenu* menu = g_MenuMgr.menu_create("", [fn](CBasePlayer* pPlayer, PlayerMenu* menu, int item) {
            if (item == MENU_EXIT || item == MENU_TIMEOUT) {
                return;
            }
            fn(detail::ImMenuCaller(menu, item), pPlayer);
        });
        fn(detail::ImMenuBuilder(menu), pPlayer);
    }
}