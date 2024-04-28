// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __TIMER_WHEEL_H__
#define __TIMER_WHEEL_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef void (*timeouthandle)(unsigned long);

void *ti_add_timer(void *ptimewheel, unsigned long expires, timeouthandle phandle, unsigned long arg);
int mod_timer(void *ptimer, unsigned long expires);
void ti_del_timer(void *p);
void *ti_timerwheel_create(void);
void ti_timerwheel_release(void *pwheel);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif