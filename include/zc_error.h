// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_ERROR_H__
#define __ZC_ERROR_H__

#include <errno.h>
#include <stddef.h>

#include "zc_macros.h"


/* error handling */
#if EDOM > 0
#define ZCERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define ZCUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define ZCERROR(e) (e)
#define ZCUNERROR(e) (e)
#endif

#define ZCERRTAG(a,b,c,d)  (-(int)ZCMKTAG(a, b, c, d))

#define ZCERR_UNKNOWN ZCERRTAG( 'U','N','K','N')
#endif
