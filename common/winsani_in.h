#if _MSC_VER >= 1500 // MSVC++ 9.0 (Visual Studio 2008)
#pragma push_macro("ARRAYSIZE")
#define HSPRITE WINDOWS_HSPRITE
#endif
#pragma push_macro("read")
#ifdef read
#undef read
#endif