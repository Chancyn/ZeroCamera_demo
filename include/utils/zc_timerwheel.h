// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_TIMERWHEEL_H__
#define __ZC_TIMERWHEEL_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
#include "zc_type.h"
#include <stdlib.h>

typedef void (*ZC_TIMER_FUNCB)(void *pdata, size_t len, void *pcontext);

struct zc_timer_info {
    ZC_S64 expires;         // expires time
    ZC_S32 timercnt;        // timer execute cnt, -1 forever ; 0 stop, 1 once; N cnt
    ZC_U32 period;          // period time for atleast once timer, if period=0; period=expires
    ZC_TIMER_FUNCB fun_cb;  // timer callback
    void *pdata;
    size_t len;
    void *pcontext;
};

void *zc_add_timer(void *ptimerwheel, const struct zc_timer_info *info);
void zc_del_timer(void *p);
void *zc_timerwheel_create(void);
void zc_timerwheel_destroy(void *ptimerwheel);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif