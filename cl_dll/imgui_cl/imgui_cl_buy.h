/*
imgui_cl_buy.h
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#ifndef CSMOE_IMGUI_CL_BUY_H
#define CSMOE_IMGUI_CL_BUY_H

#include <memory>

class CImGuiBuyMenu {
public:
    CImGuiBuyMenu();
    ~CImGuiBuyMenu();
    void OnGUI();
private:
    struct impl_t;
    const std::unique_ptr<impl_t> pimpl;
};

#endif //CSMOE_IMGUI_CL_BUY_H
