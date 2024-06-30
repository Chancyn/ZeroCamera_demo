// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_PROC_H__
#define __ZC_PROC_H__
#include <errno.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef TASK_COMM_LEN
#define ZC_TASK_COMM_LEN TASK_COMM_LEN
#else
#define ZC_TASK_COMM_LEN (16)
#endif

const char *g_buildDateTime = __DATE__ " " __TIME__;

#define ZC_PROC_ABSPATH(p, len) \
    ({ \
        int i = 0; \
        char path[128] = {0}; \
        int cnt = readlink("/proc/self/exe", path, 128); \
        if (cnt > 0 && cnt < 128) { \
            for (i = cnt; i >= 0; --i) { \
                if (path[i] == '/') { \
                    path[i + 1] = '\0'; \
                    break; \
                } \
            } \
        } \
        strncpy(p, path, len - 1); \
    })



#define ZC_PROC_SETNAME(n) prctl(PR_SET_NAME, (unsigned long)n, 0, 0, 0);  // NOLINT

#if 1
static inline void zc_proc_getname(char *p, int len) {
    FILE *fp;
    fp = fopen("/proc/self/comm", "r");
    if (fp) {
        fread(p, 1, len, fp);
        p[strcspn(p, "\n")] = '\0';
        printf("get pname ok :%s\n", p);
        fclose(fp);
    } else {
        printf("get pname error errno:%d(%s)\n", errno, strerror(errno));
    }
    return;
}
#define ZC_PROC_GETNAME(p, len) zc_proc_getname(p, len)
#else
#define ZC_PROC_GETNAME(p, len) \
    ({ \
        char name[ZC_TASK_COMM_LEN] = {0}; \
        int ret = readlink("/proc/self/comm", name, ZC_TASK_COMM_LEN); \
        if (ret > 0) { \
            printf("get pname ok ret:%d,%s\n", ret, name); \
            name[ret] = '\0'; \
            strncpy(p, name, len - 1); \
        } else { \
            printf("get pname error ret:%d, errno:%d(%s)\n", ret, errno, strerror(errno)); \
        } \
    })
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
