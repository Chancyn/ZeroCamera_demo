// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from zlib-1.3.1/zlib.h,zutil.h


#ifndef __ZC_CRC32_H__
#define __ZC_CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// copy from zconf.h
#ifdef __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  if __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#endif
#if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#endif
#if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#endif
#if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#endif
#if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#endif

#if defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#endif

#ifndef STDC
#  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define const       /* note: need a more gentle solution here */
#  endif
#endif

#if defined(ZLIB_CONST) && !defined(z_const)
#  define z_const const
#else
#  define z_const
#endif

#ifdef Z_SOLO
#  ifdef _WIN64
     typedef unsigned long long z_size_t;
#  else
     typedef unsigned long z_size_t;
#  endif
#else
#  define z_longlong long long
#  if defined(NO_SIZE_T)
     typedef unsigned NO_SIZE_T z_size_t;
#  elif defined(STDC)
#    include <stddef.h>
     typedef size_t z_size_t;
#  else
     typedef unsigned long z_size_t;
#  endif
#  undef z_longlong
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus about 7 kilobytes
 for small objects.
*/

                        /* Type declarations */

#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ifdef SYS16BIT
#  if defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#  if (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#endif

#if defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_INTERNAL
#        define ZEXTERN extern __declspec(dllexport)
#      else
#        define ZEXTERN extern __declspec(dllimport)
#      endif
#    endif
#  endif  /* ZLIB_DLL */
   /* If building or using zlib with the WINAPI/WINAPIV calling convention,
    * define ZLIB_WINAPI.
    * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
    */
#  ifdef ZLIB_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    ifndef WIN32_LEAN_AND_MEAN
#      define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    else
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#endif

#if defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_INTERNAL
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    else
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif
#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif

#ifndef FAR
#  define FAR
#endif

#if !defined(__MACTYPES__)
typedef unsigned char  Byte;  /* 8 bits */
#endif
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

#ifdef SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
   typedef void const *voidpc;
   typedef void FAR   *voidpf;
   typedef void       *voidp;
#else
   typedef Byte const *voidpc;
   typedef Byte FAR   *voidpf;
   typedef Byte       *voidp;
#endif

#if !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  if (UINT_MAX == 0xffffffffUL)
#    define Z_U4 unsigned
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 unsigned long
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 unsigned short
#  endif
#endif

#ifdef Z_U4
   typedef Z_U4 z_crc_t;
#else
   typedef unsigned long z_crc_t;
#endif

#if 1    /* was set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#endif

#if 1    /* was set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#endif

#ifdef STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#endif

#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#endif

#ifdef _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#endif

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#if defined(_LARGEFILE64_SOURCE) && -_LARGEFILE64_SOURCE - -1 == 1
#  undef _LARGEFILE64_SOURCE
#endif

#ifndef Z_HAVE_UNISTD_H
#  ifdef __WATCOMC__
#    define Z_HAVE_UNISTD_H
#  endif
#endif
#ifndef Z_HAVE_UNISTD_H
#  if defined(_LARGEFILE64_SOURCE) && !defined(_WIN32)
#    define Z_HAVE_UNISTD_H
#  endif
#endif
#ifndef Z_SOLO
#  if defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#endif

#if defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#endif

#if defined(_LARGEFILE64_SOURCE) && defined(Z_LFS64)
#  define Z_LARGE64
#endif

#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#endif

#if !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif

#ifndef z_off_t
#  define z_off_t long
#endif

#if !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#else
#  if defined(_WIN32) && !defined(__GNUC__)
#    define z_off64_t __int64
#  else
#    define z_off64_t z_off_t
#  endif
#endif

// copy from zutil.h
#ifndef local
#  define local static
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

#if !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  if (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

// copy from zlib
#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

ZEXTERN uLong ZEXPORT crc32(uLong crc, const Bytef *buf, uInt len);
/*
     Update a running CRC-32 with the bytes buf[0..len-1] and return the
   updated CRC-32. A CRC-32 value is in the range of a 32-bit unsigned integer.
   If buf is Z_NULL, this function returns the required initial value for the
   crc. Pre- and post-conditioning (one's complement) is performed within this
   function so it shouldn't be done by the application.

   Usage example:

     uLong crc = crc32(0L, Z_NULL, 0);

     while (read_buffer(buffer, length) != EOF) {
       crc = crc32(crc, buffer, length);
     }
     if (crc != original_crc) error();
*/

ZEXTERN uLong ZEXPORT crc32_z(uLong crc, const Bytef *buf,
                              z_size_t len);
/*
     Same as crc32(), but with a size_t length.
*/

/*
ZEXTERN uLong ZEXPORT crc32_combine(uLong crc1, uLong crc2, z_off_t len2);

     Combine two CRC-32 check values into one.  For two sequences of bytes,
   seq1 and seq2 with lengths len1 and len2, CRC-32 check values were
   calculated for each, crc1 and crc2.  crc32_combine() returns the CRC-32
   check value of seq1 and seq2 concatenated, requiring only crc1, crc2, and
   len2. len2 must be non-negative.
*/

/*
ZEXTERN uLong ZEXPORT crc32_combine_gen(z_off_t len2);

     Return the operator corresponding to length len2, to be used with
   crc32_combine_op(). len2 must be non-negative.
*/

ZEXTERN uLong ZEXPORT crc32_combine_op(uLong crc1, uLong crc2, uLong op);
/*
     Give the same result as crc32_combine(), using op in place of len2. op is
   is generated from len2 by crc32_combine_gen(). This will be faster than
   crc32_combine() if the generated op is used more than once.
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __ZC_MD5_H__ */
