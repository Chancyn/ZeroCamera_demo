// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/*
 *毫秒定时器  采用多级时间轮方式  借鉴linux内核中的实现
 *支持的范围为1 ~  2^32 毫秒(大约有49天)
 *若设置的定时器超过最大值 则按最大值设置定时器
 **/
#include <asm-generic/errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "zc_list.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "zc_log.h"
#include "zc_timerwheel.h"

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)

#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define SEC_VALUE 0
#define USEC_VALUE 2000

// clock_gettime
#define ZC_TIMER_CLOCK_GETTIME 1
#ifdef ZC_TIMER_CLOCK_GETTIME

#endif

struct tvec_base;
#define INDEX(N) ((base->current_index >> (TVR_BITS + (N)*TVN_BITS)) & TVN_MASK)

struct timer_list {
    struct list_head entry;  // 将时间连接成链表
    struct tvec_base *base;  // 指向时间轮
    ZC_U64 expires;          // expires time
    ZC_S32 timercnt;         // timer execute cnt, -1 forever ; 0 stop, 1 once; N cnt
    ZC_U32 period;           // period timer for atleast once timer, if period=0; period=expires
    ZC_TIMER_FUNCB fun_cb;   // timer callback
    void *pdata;
    size_t len;
    void *pcontext;
};

struct tvec {
    struct list_head vec[TVN_SIZE];
};

struct tvec_root {
    struct list_head vec[TVR_SIZE];
};

// 实现5级时间轮 范围为0~ (2^8 * 2^6 * 2^6 * 2^6 *2^6)=2^32
struct tvec_base {
    bool running;
    ZC_U64 current_index;
    pthread_spinlock_t lock;
    // pthread_t thincrejiffies;
    pthread_t tid;
    struct tvec_root tv1; /*第一个轮*/
    struct tvec tv2;      /*第二个轮*/
    struct tvec tv3;      /*第三个轮*/
    struct tvec tv4;      /*第四个轮*/
    struct tvec tv5;      /*第五个轮*/
};

static void internal_add_timer(struct tvec_base *base, struct timer_list *timer) {
    struct list_head *vec;
    ZC_U64 expires = timer->expires;
    ZC_S64 idx = expires - base->current_index;

    /*这里是没有办法区分出是过时还是超长定时的吧?*/
    if ((ZC_S64)idx < 0) {
        /*放到第一个轮的当前槽*/
        vec = base->tv1.vec + (base->current_index & TVR_MASK);
    } else if (idx < TVR_SIZE) {
        /*第一个轮*/
        int i = expires & TVR_MASK;
        vec = base->tv1.vec + i;
    } else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
        /*第二个轮*/
        int i = (expires >> TVR_BITS) & TVN_MASK;
        vec = base->tv2.vec + i;
    } else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
        /*第三个轮*/
        int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
        vec = base->tv3.vec + i;
    } else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
        /*第四个轮*/
        int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
        vec = base->tv4.vec + i;
    } else {
        /*第五个轮*/
        int i;
        if (idx > 0xffffffffUL) {
            idx = 0xffffffffUL;
            expires = idx + base->current_index;
        }
        i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        vec = base->tv5.vec + i;
    }

    list_add_tail(&timer->entry, vec);
}

static inline void _detach_timer(struct timer_list *timer) {
    struct list_head *entry = &timer->entry;
    ZC_ASSERT(entry->prev != NULL);
    ZC_ASSERT(entry->next != NULL);
    list_del(&timer->entry);
#if 0
    struct list_head *entry = &timer->entry;
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
#endif
}

static int internal_mod_timer(struct timer_list *timer, unsigned long expires) {
    if (NULL != timer->entry.next)
        _detach_timer(timer);

    internal_add_timer(timer->base, timer);

    return 0;
}

// private
static int _mod_timer(void *ptimer, ZC_U64 expires) {
    struct timer_list *timer = (struct timer_list *)ptimer;
    struct tvec_base *base;

    base = timer->base;
    if (NULL == base)
        return -1;

    // calcu expires
    expires = expires + base->current_index;
    if (timer->entry.next != NULL && timer->expires == expires) {
        LOG_ERROR("timer _mod_timer error expires[%llu]", timer->expires);
        return 0;
    }

    timer->expires = expires;
    internal_mod_timer(timer, expires);
    // LOG_TRACE("timer internal_mod_timer expires[%llu]", timer->expires);
    return 0;
}

// 添加一个定时器
static void __zc_add_timer(struct timer_list *timer, ZC_U64 expires) {
    if (NULL != timer->entry.next) {
        LOG_ERROR("timer is already exist");
        return;
    }

    _mod_timer(timer, expires);
}

// 添加一个定时器  外部接口 返回定时器
// void *zc_add_timer(void *ptimerwheel, unsigned long expires, function phandle, unsigned long arg)
void *zc_add_timer(void *ptimerwheel, const struct zc_timer_info *info) {
    if (NULL == ptimerwheel || NULL == info || NULL == info->fun_cb) {
        LOG_ERROR("add timer error, args error");
        return NULL;
    }

    struct timer_list *ptimer;

    ptimer = (struct timer_list *)malloc(sizeof(struct timer_list));
    if (NULL == ptimer) {
        return NULL;
    }

    memset(ptimer, 0, sizeof(struct timer_list));
    ptimer->entry.next = NULL;
    ptimer->base = (struct tvec_base *)ptimerwheel;
    ptimer->expires = info->expires;
    ptimer->period = info->period ? info->expires : info->expires;  // if period=0; period=expires
    ptimer->timercnt = info->timercnt;
    // ptimer->timercnt = (info->timercnt == 0) ? 1 : info->timercnt;  // atleast execute once
    ptimer->fun_cb = info->fun_cb;
    ptimer->pdata = info->pdata;
    ptimer->len = info->len;
    ptimer->pcontext = info->pcontext;

    LOG_TRACE("add timer timercnt[%d],[%llu] [%p]", info->timercnt, info->expires, ptimer);
    pthread_spin_lock(&ptimer->base->lock);
    __zc_add_timer(ptimer, ptimer->expires);
    pthread_spin_unlock(&ptimer->base->lock);
    return ptimer;
}

// private find timer
static bool _find_timer_list(struct timer_list *timer) {
    LOG_ERROR("_find_timer_list into [%p]", timer);
    struct list_head *pos, *tmp;
    struct timer_list *pen;

    int i = 0;
    for (i = 0; i < TVR_SIZE; i++) {
        struct list_head *head = &timer->base->tv1.vec[i];
        list_for_each_safe(pos, tmp, head) {
            pen = list_entry(pos, struct timer_list, entry);
            if (pen == timer) {
                LOG_WARN("_find_timer_list tv1[%d] timercnt[%d] [%p] head[%p]", i, timer->timercnt, timer, pos);
                return true;
            }
        }
    }

    for (i = 0; i < TVN_SIZE; i++) {
        struct list_head *head = &timer->base->tv2.vec[i];
        list_for_each_safe(pos, tmp, head) {
            pen = list_entry(pos, struct timer_list, entry);
            if (pen == timer) {
                LOG_WARN("_find_timer_list tv2[%d] timercnt[%d] [%p] head[%p]", i, timer->timercnt, timer, pos);
                return true;
            }
        }
    }

    for (i = 0; i < TVN_SIZE; i++) {
        struct list_head *head = &timer->base->tv3.vec[i];
        list_for_each_safe(pos, tmp, head) {
            pen = list_entry(pos, struct timer_list, entry);
            if (pen == timer) {
                LOG_WARN("_find_timer_list tv3[%d] timercnt[%d] [%p] head[%p]", i, timer->timercnt, timer, pos);
                return true;
            }
        }
    }

    for (i = 0; i < TVN_SIZE; i++) {
        struct list_head *head = &timer->base->tv4.vec[i];
        list_for_each_safe(pos, tmp, head) {
            pen = list_entry(pos, struct timer_list, entry);
            if (pen == timer) {
                LOG_WARN("_find_timer_list tv4[%d] timercnt[%d] [%p] head[%p]", i, timer->timercnt, timer, pos);
                return true;
            }
        }
    }

    for (i = 0; i < TVN_SIZE; i++) {
        struct list_head *head = &timer->base->tv5.vec[i];
        list_for_each_safe(pos, tmp, head) {
            pen = list_entry(pos, struct timer_list, entry);
            if (pen == timer) {
                LOG_WARN("_find_timer_list tv5[%d] timercnt [%d] [%p] head[%p]", i, timer->timercnt, timer, pos);
                return true;
            }
        }
    }

    LOG_TRACE("_find_timer_list not find [%p]", timer);
    return NULL;
}

// private del timer for api,
static void _zc_del_timer(struct timer_list *timer) {
    // check timer exist, find and delete
    if (_find_timer_list(timer)) {
        LOG_WARN("_find_timer_list find [%p]", timer);
        list_del(&timer->entry);
        free(timer);  // free
    }
    return;
}

// public del timer
void zc_del_timer(void *p) {
    struct timer_list *ptimer = (struct timer_list *)p;

    if (NULL == ptimer)
        return;

    pthread_spin_lock(&ptimer->base->lock);
    _zc_del_timer(ptimer);
    pthread_spin_unlock(&ptimer->base->lock);

    LOG_TRACE("del timer ok");
    return;
}

// 时间轮级联
static int cascade(struct tvec_base *base, struct tvec *tv, int index) {
    struct list_head *pos, *tmp;
    struct timer_list *timer;
    struct list_head tv_list;

    // 将tv[index]槽位上的所有任务转移给tv_list,然后清空tv[index]
    // 用tv_list替换tv->vec + index
    list_replace_init(tv->vec + index, &tv_list);

    // 遍历tv_list双向链表，将任务重新添加到时间轮
    list_for_each_safe(pos, tmp, &tv_list) {
        timer = list_entry(pos, struct timer_list, entry);
        internal_add_timer(base, timer);
    }

    return index;
}

static void timer_process_run(struct tvec_base *base) {
    struct timer_list *timer;

    ZC_U64 current_index = 0;

    #ifdef ZC_TIMER_CLOCK_GETTIME
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    current_index = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
    #else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    current_index = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
    #endif

    pthread_spin_lock(&base->lock);
    while (base->running && base->current_index <= current_index) {
        struct list_head work_list;
        // 获取第一个轮上的指针位置
        ZC_U64 index = base->current_index & TVR_MASK;
        struct list_head *head = &work_list;
        // 指针指向0槽时，级联轮需要更新任务列表
        if (!index && (!cascade(base, &base->tv2, INDEX(0))) && (!cascade(base, &base->tv3, INDEX(1))) &&
            (!cascade(base, &base->tv4, INDEX(2))))
            cascade(base, &base->tv5, INDEX(3));

        base->current_index++;
        list_replace_init(base->tv1.vec + index, &work_list);
        while (!list_empty(head)) {
            timer = list_first_entry(head, struct timer_list, entry);
            _detach_timer(timer);
            timer->fun_cb(timer->pdata, timer->len, timer->pcontext);
            if (timer->timercnt < 0 || (--timer->timercnt > 0)) {
                // LOG_WARN("_add timer again, cnt[%d] expires[%llu] [%p]", timer->timercnt, timer->expires, timer);
                __zc_add_timer(timer, timer->period);
            } else {
                // LOG_WARN("_stop timer, timercnt[%d] expires[%llu] [%p]", timer->timercnt, timer->expires, timer);
                free(timer);
            }
        }
    }
    pthread_spin_unlock(&base->lock);

    return;
}

static void *timer_process(void *args) {
    struct tvec_base *base = (struct tvec_base *)args;
    LOG_INFO("timer_process into");
    while (base->running) {
        timer_process_run(base);
        ZC_MSLEEP(5);
    }

    LOG_INFO("timer_process exit");
    return NULL;
}

static void init_tvr_list(struct tvec_root *tvr) {
    int i;

    for (i = 0; i < TVR_SIZE; i++)
        INIT_LIST_HEAD(&tvr->vec[i]);
}

static void init_tvn_list(struct tvec *tvn) {
    int i;

    for (i = 0; i < TVN_SIZE; i++)
        INIT_LIST_HEAD(&tvn->vec[i]);
}

// public create timerwheel
void *zc_timerwheel_create(void) {
    struct tvec_base *base;

    base = (struct tvec_base *)malloc(sizeof(struct tvec_base));
    if (NULL == base) {
        LOG_ERROR("timerwheel malloc error");
        goto _err_free;
    }

    memset(base, 0, sizeof(struct tvec_base));

    init_tvr_list(&base->tv1);
    init_tvn_list(&base->tv2);
    init_tvn_list(&base->tv3);
    init_tvn_list(&base->tv4);
    init_tvn_list(&base->tv5);
    #ifdef ZC_TIMER_CLOCK_GETTIME
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    base->current_index = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
    #else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    base->current_index = tv.tv_sec * 1000 + tv.tv_usec / 1000; /*当前时间毫秒数*/
    #endif
    pthread_spin_init(&base->lock, PTHREAD_PROCESS_PRIVATE);
    base->running = true;
    if (0 != pthread_create(&base->tid, NULL, timer_process, base)) {
        base->running = false;
        LOG_ERROR("timerwheel create error, pthread_create error");
        goto _err_free;
    }

    LOG_INFO("timerwheel create ok");
    return base;
_err_free:
    ZC_SAFE_FREE(base);
    LOG_ERROR("timerwheel create error");
    return NULL;
}

static void zc_uninit_tvr(struct tvec_root *pvr) {
    int i;
    struct list_head *pos, *tmp;
    struct timer_list *pen;

    for (i = 0; i < TVR_SIZE; i++) {
        list_for_each_safe(pos, tmp, &pvr->vec[i]) {
            pen = list_entry(pos, struct timer_list, entry);
            list_del(pos);
            free(pen);
        }
    }
}

static void zc_uninit_tvn(struct tvec *pvn) {
    int i;
    struct list_head *pos, *tmp;
    struct timer_list *pen;

    for (i = 0; i < TVN_SIZE; i++) {
        list_for_each_safe(pos, tmp, &pvn->vec[i]) {
            pen = list_entry(pos, struct timer_list, entry);
            list_del(pos);
            free(pen);
        }
    }
}

// public destroy timerwheel
void zc_timerwheel_destroy(void *ptimerwheel) {
    struct tvec_base *base = (struct tvec_base *)ptimerwheel;

    if (NULL == base)
        return;
    LOG_INFO("timerwheel destroy");
    if (base->running) {
        base->running = false;
        pthread_join(base->tid, NULL);
        base->tid = 0;
    }

    zc_uninit_tvr(&base->tv1);
    zc_uninit_tvn(&base->tv2);
    zc_uninit_tvn(&base->tv3);
    zc_uninit_tvn(&base->tv4);
    zc_uninit_tvn(&base->tv5);
    pthread_spin_destroy(&base->lock);

    free(base);
    LOG_INFO("timerwheel destroy ok");
    return;
}