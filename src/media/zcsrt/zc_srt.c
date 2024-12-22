// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// svr
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <srt/srt.h>

#include "zc_macros.h"
#include "zc_log.h"
#include "zc_srt.h"
#include "zc_error.h"

#include "zc_basic_fun.h"

ZC_UNUSED static int zc_srt_neterrno() {
    int os_errno;
    int err = srt_getlasterror(&os_errno);
    if (err == SRT_EASYNCRCV || err == SRT_EASYNCSND)
        return ZCERROR(EAGAIN);
    LOG_ERROR("%s", srt_getlasterror_str());
    return os_errno ? ZCERROR(os_errno) : ZCERR_UNKNOWN;
}

int64_t av_gettime_relative(void)
{
    return zc_system_clock();
}