/*
nodiscard.h - Nodiscard declaration
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_NODISCARD_H
#define PROJECT_NODISCARD_H

#if _MSC_VER
#   if _CRT_HAS_CXX17
#       define NODISCARD [[nodiscard]]
#   endif
#elif __cplusplus
#   if __cplusplus >= 201703L && !defined(NODISCARD)
#       define NODISCARD [[nodiscard]]
#   endif
#endif

#ifndef NODISCARD
# define NODISCARD
#endif

#endif //PROJECT_NODISCARD_H
