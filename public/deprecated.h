/*
deprecated.h - Deprecated declaration
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

#ifndef PROJECT_DEPRECATED_H
#define PROJECT_DEPRECATED_H

// Function is deprecated
#ifdef __cplusplus
#   if __cplusplus >= 201402L && !defined(DEPRECATED)
#       define DEPRECATED [[deprecated]]
#   endif
#endif
#ifndef DEPRECATED
#   if defined(__GNUC__)
#       define DEPRECATED __attribute__((deprecated))
#   elif defined(_MSC_VER)
#       define DEPRECATED __declspec(deprecated)
#   else
#       define DEPRECATED
#   endif
#endif

#endif //PROJECT_DEPRECATED_H
