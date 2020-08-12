#pragma once

#include <string>

namespace ui {
    class IGameDialog {
    public:
        virtual ~IGameDialog() = 0;

        virtual void OnGUI() = 0;
    };

    inline IGameDialog::~IGameDialog() = default;
}