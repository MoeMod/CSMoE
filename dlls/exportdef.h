#ifndef EXPORTDEF_H
#define EXPORTDEF_H

#ifdef EXPORT
#undef EXPORT
#endif
#ifdef DLLEXPORT
#undef DLLEXPORT
#endif

#if defined _WIN32 || defined __CYGWIN__
	#ifdef __GNUC__
		#define EXPORT __attribute__ ((dllexport))
	#else
		#define EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
	#endif
#else
  #if __GNUC__ >= 4
	#define EXPORT __attribute__ ((visibility ("default")))
   #else
	#define EXPORT
  #endif
#endif
#define DLLEXPORT EXPORT
#define _DLLEXPORT EXPORT

#define C_DLLEXPORT extern "C" DLLEXPORT
#define EXT_FUNC /*FORCE_STACK_ALIGN*/

#endif // EXPORTDEF_H
