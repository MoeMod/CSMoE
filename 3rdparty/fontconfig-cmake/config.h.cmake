/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#cmakedefine AC_APPLE_UNIVERSAL_BUILD @AC_APPLE_UNIVERSAL_BUILD@

/* The normal alignment of `void *', in bytes. */
#cmakedefine ALIGNOF_VOID_P @ALIGNOF_VOID_P@

/* The normal alignment of `double', in bytes. */
#cmakedefine ALIGNOF_DOUBLE @ALIGNOF_DOUBLE@

/* Use libxml2 instead of Expat */
#cmakedefine ENABLE_LIBXML2 @ENABLE_LIBXML2@

/* Additional font directories */
#cmakedefine FC_ADD_FONTS "yes"

/* Architecture prefix to use for cache file names */
#cmakedefine FC_ARCHITECTURE @FC_ARCHITECTURE@

/* System font directory */
#define FC_DEFAULT_FONTS @FC_DEFAULT_FONTS@

/* The type of len parameter of the gperf hash/lookup function */
#define FC_GPERF_SIZE_T @FC_GPERF_SIZE_T@

/* temporary cachedir */
#define FC_CACHEDIR @FC_CACHEDIR@

/* Define to nothing if C supports flexible array members, and to 1 if it does
not. That way, with a declaration like `struct s { int n; double
d[FLEXIBLE_ARRAY_MEMBER]; };', the struct hack can be used with pre-C99
compilers. When computing the size of such an object, don't use 'sizeof
(struct s)' as it overestimates the size. Use 'offsetof (struct s, d)'
instead. Don't use 'offsetof (struct s, d[0])', as this doesn't work with
MSVC and with C++ compilers. */
#define FLEXIBLE_ARRAY_MEMBER

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
*/
#cmakedefine HAVE_DIRENT_H @HAVE_DIRENT_H@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H @HAVE_DLFCN_H@

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#cmakedefine HAVE_DOPRNT @HAVE_DOPRNT@

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H @HAVE_FCNTL_H@

/* Define to 1 if you have the `fstatfs' function. */
#cmakedefine HAVE_FSTATFS @HAVE_FSTATFS@

/* Define to 1 if you have the `fstatvfs' function. */
#cmakedefine HAVE_FSTATVFS @HAVE_FSTATVFS@

/* FT_Bitmap_Size structure includes y_ppem field */
#cmakedefine HAVE_FT_BITMAP_SIZE_Y_PPEM

/* Define to 1 if you have the `FT_Get_BDF_Property' function. */
#cmakedefine HAVE_FT_GET_BDF_PROPERTY @HAVE_FT_GET_BDF_PROPERTY@

/* Define to 1 if you have the `FT_Get_Next_Char' function. */
#cmakedefine HAVE_FT_GET_NEXT_CHAR @HAVE_FT_GET_NEXT_CHAR@

/* Define to 1 if you have the `FT_Get_PS_Font_Info' function. */
#cmakedefine HAVE_FT_GET_PS_FONT_INFO @HAVE_FT_GET_PS_FONT_INFO@

/* Define to 1 if you have the `FT_Get_X11_Font_Format' function. */
#cmakedefine HAVE_FT_GET_X11_FONT_FORMAT @HAVE_FT_GET_X11_FONT_FORMAT@

/* Define to 1 if you have the `FT_Has_PS_Glyph_Names' function. */
#cmakedefine HAVE_FT_HAS_PS_GLYPH_NAMES @HAVE_FT_HAS_PS_GLYPH_NAMES@

/* Define to 1 if you have the `FT_Select_Size' function. */
#cmakedefine HAVE_FT_SELECT_SIZE @HAVE_FT_SELECT_SIZE@

/* Define to 1 if you have the `getexecname' function. */
#cmakedefine HAVE_GETEXECNAME @HAVE_GETEXECNAME@

/* Define to 1 if you have the `getopt' function. */
#cmakedefine HAVE_GETOPT @HAVE_GETOPT@

/* Define to 1 if you have the `getopt_long' function. */
#cmakedefine HAVE_GETOPT_LONG @HAVE_GETOPT_LONG@

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine HAVE_GETPAGESIZE @HAVE_GETPAGESIZE@

/* Define to 1 if you have the `getprogname' function. */
#cmakedefine HAVE_GETPROGNAME @HAVE_GETPROGNAME@

/* Have Intel __sync_* atomic primitives */
#cmakedefine HAVE_INTEL_ATOMIC_PRIMITIVES @HAVE_INTEL_ATOMIC_PRIMITIVES@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H @HAVE_INTTYPES_H@

/* Define to 1 if you have the `link' function. */
#cmakedefine HAVE_LINK @HAVE_LINK@

/* Define to 1 if you have the `lrand48' function. */
#cmakedefine HAVE_LRAND48 @HAVE_LRAND48@

/* Define to 1 if you have the `lstat' function. */
#cmakedefine HAVE_LSTAT @HAVE_LSTAT@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H @HAVE_MEMORY_H@

/* Define to 1 if you have the `mkdtemp' function. */
#cmakedefine HAVE_MKDTEMP @HAVE_MKDTEMP@

/* Define to 1 if you have the `mkostemp' function. */
#cmakedefine HAVE_MKOSTEMP @HAVE_MKOSTEMP@

/* Define to 1 if you have the `mkstemp' function. */
#cmakedefine HAVE_MKSTEMP @HAVE_MKSTEMP@

/* Define to 1 if you have a working `mmap' system call. */
#cmakedefine HAVE_MMAP @HAVE_MMAP@

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H @HAVE_NDIR_H@

/* Define to 1 if you have the 'posix_fadivse' function. */
#cmakedefine HAVE_POSIX_FADVISE @HAVE_POSIX_FADVISE@

/* Have POSIX threads */
#cmakedefine HAVE_PTHREAD @HAVE_PTHREAD@

/* Have PTHREAD_PRIO_INHERIT. */
#cmakedefine HAVE_PTHREAD_PRIO_INHERIT @HAVE_PTHREAD_PRIO_INHERIT@

/* Define to 1 if you have the `rand' function. */
#cmakedefine HAVE_RAND @HAVE_RAND@

/* Define to 1 if you have the `random' function. */
#cmakedefine HAVE_RANDOM @HAVE_RANDOM@

/* Define to 1 if you have the `random_r' function. */
#cmakedefine HAVE_RANDOM_R @HAVE_RANDOM_R@

/* Define to 1 if you have the `rand_r' function. */
#cmakedefine HAVE_RAND_R @HAVE_RAND_R@

/* Define to 1 if you have the `readlink' function. */
#cmakedefine HAVE_READLINK @HAVE_READLINK@

/* Define to 1 if you have the `regcomp' function. */
#cmakedefine HAVE_REGCOMP @HAVE_REGCOMP@

/* Define to 1 if you have the `regerror' function. */
#cmakedefine HAVE_REGERROR @HAVE_REGERROR@

/* Define to 1 if you have the `regexec' function. */
#cmakedefine HAVE_REGEXEC @HAVE_REGEXEC@

/* Define to 1 if you have the <regex.h> header file. */
#cmakedefine HAVE_REGEX_H @HAVE_REGEX_H@

/* Define to 1 if you have the `regfree' function. */
#cmakedefine HAVE_REGFREE @HAVE_REGFREE@

/* Define to 1 if you have the 'scandir' function. */
#cmakedefine HAVE_SCANDIR @HAVE_SCANDIR@

/* Define to 1 if you have the 'scandir' function with int (* compar)(const
void *, const void *) */
#cmakedefine HAVE_SCANDIR_VOID_P @HAVE_SCANDIR_VOID_P@

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H @HAVE_SCHED_H@

/* Have sched_yield */
#cmakedefine HAVE_SCHED_YIELD @HAVE_SCHED_YIELD@

/* Have Solaris __machine_*_barrier and atomic_* operations */
#cmakedefine HAVE_SOLARIS_ATOMIC_OPS @HAVE_SOLARIS_ATOMIC_OPS@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H @HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H @HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H @HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H @HAVE_STRING_H@

/* Define to 1 if `d_type' is a member of `struct dirent'. */
#cmakedefine HAVE_STRUCT_DIRENT_D_TYPE

/* Define to 1 if `f_flags' is a member of `struct statfs'. */
#cmakedefine HAVE_STRUCT_STATFS_F_FLAGS

/* Define to 1 if `f_fstypename' is a member of `struct statfs'. */
#cmakedefine HAVE_STRUCT_STATFS_F_FSTYPENAME

/* Define to 1 if `f_basetype' is a member of `struct statvfs'. */
#cmakedefine HAVE_STRUCT_STATVFS_F_BASETYPE

/* Define to 1 if `f_fstypename' is a member of `struct statvfs'. */
#cmakedefine HAVE_STRUCT_STATVFS_F_FSTYPENAME

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
*/
#cmakedefine HAVE_SYS_DIR_H @HAVE_SYS_DIR_H@

/* Define to 1 if you have the <sys/mount.h> header file. */
#cmakedefine HAVE_SYS_MOUNT_H @HAVE_SYS_MOUNT_H@

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
*/
#cmakedefine HAVE_SYS_NDIR_H @HAVE_SYS_NDIR_H@

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H @HAVE_SYS_PARAM_H@

/* Define to 1 if you have the <sys/statfs.h> header file. */
#cmakedefine HAVE_SYS_STATFS_H @HAVE_SYS_STATFS_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H @HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H @HAVE_SYS_TYPES_H@

/* Define to 1 if you have the <sys/vfs.h> header file. */
#cmakedefine HAVE_SYS_VFS_H @HAVE_SYS_VFS_H@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H @HAVE_UNISTD_H@

/* Define to 1 if you have the `vprintf' function. */
#cmakedefine HAVE_VPRINTF @HAVE_VPRINTF@

/* Can use #warning in C files */
#cmakedefine HAVE_WARNING_CPP_DIRECTIVE @HAVE_WARNING_CPP_DIRECTIVE@

/* Use xmlparse.h instead of expat.h */
#cmakedefine HAVE_XMLPARSE_H @HAVE_XMLPARSE_H@

/* Define to 1 if you have the `XML_SetDoctypeDeclHandler' function. */
#cmakedefine HAVE_XML_SETDOCTYPEDECLHANDLER @HAVE_XML_SETDOCTYPEDECLHANDLER@

/* Define to 1 if you have the `_mktemp_s' function. */
#cmakedefine HAVE__MKTEMP_S @HAVE__MKTEMP_S@

/* Define to the sub-directory in which libtool stores uninstalled libraries.
*/
#cmakedefine LT_OBJDIR

/* Name of package */
#define PACKAGE "fontconfig"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#define PACKAGE_NAME "fontconfig"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "fontconfig"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION 1

/* Define to necessary symbol if this constant
uses a non-standard name on
your system. */
#cmakedefine PTHREAD_CREATE_JOINABLE

/* The size of `char', as computed by sizeof. */
#cmakedefine SIZEOF_CHAR @SIZEOF_CHAR@

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT @SIZEOF_INT@

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG @SIZEOF_LONG@

/* The size of `short', as computed by sizeof. */
#cmakedefine SIZEOF_SHORT @SIZEOF_SHORT@

/* The size of `void*', as computed by sizeof. */
#cmakedefine SIZEOF_VOIDP @SIZEOF_VOIDP@

/* The size of `void *', as computed by sizeof. */
#cmakedefine SIZEOF_VOID_P @SIZEOF_VOID_P@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@

/* Use iconv. */
#cmakedefine USE_ICONV @USE_ICONV@

/* Use regex */
#cmakedefine USE_REGEX @USE_REGEX@

/* Enable extensions on AIX 3, Interix.  */
#cmakedefine _ALL_SOURCE

/* Enable GNU extensions on systems that have them.  */
#cmakedefine _GNU_SOURCE

/* Enable threading extensions on Solaris.  */
#define _POSIX_PTHREAD_SEMANTICS @_POSIX_PTHREAD_SEMANTICS@

/* Enable extensions on HP NonStop.  */
#cmakedefine _TANDEM_SOURCE

/* Enable general extensions on Solaris.  */
#define __EXTENSIONS__ @__EXTENSIONS__@

/* Version number of package */
#cmakedefine VERSION

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#cmakedefine _FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
#cmakedefine _LARGE_FILES

/* Define to 1 if on MINIX. */
#cmakedefine _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
this defined. */
#cmakedefine _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine _POSIX_SOURCE @_POSIX_SOURCE@

/* Define to empty if `const' does not conform to ANSI C. */
#cmakedefine const

/* Define to `__inline__' or `__inline' if that's what the C compiler
calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine pid_t