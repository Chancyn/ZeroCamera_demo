#include <stdio.h>

#include "zc_log.h"
#include "zc_proc.h"

const char *g_buildDateTime = __DATE__ " " __TIME__;
const char *g_Version = ZC_VERSION;
const char *g_VersionbuildDate = ZC_VERSION "(" __DATE__ " " __TIME__ ")";
unsigned int g_Init = 0;
unsigned int g_VersionNum = 0;

const char *ZcGetVersionStr() {
    return g_Version;
}

const char *ZcGetBuildDateTimeStr() {
    return g_buildDateTime;
}

const char *ZcGetVersionBuildDateStr() {
    return g_VersionbuildDate;
}

unsigned int VersionStr2Num(const char *str) {
    if (str == NULL) {
        return -1;
    }

    int a, b, c, d;
    unsigned int version = 0;
    int ret = sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
    if (ret == 4) {
        version = ((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((c & 0xFF) << 8) | (d & 0xFF);
        LOG_TRACE("Version parse:%s->0x%0x\n", version);
    } else {
        LOG_ERROR("prase error:%s,ret:%d", str, ret);
    }

    return version;
}

char *VersionNum2str(char *str, unsigned int len, unsigned int num) {
    if (str == NULL || len == 0) {
        return NULL;
    }

    snprintf(str, len, "%u.%u.%u.%u", (num >> 24) & 0xFF, (num >> 16) & 0xFF, (num >> 8) & 0xFF, num & 0xFF);

    return str;
}

unsigned int ZcGetVersionNum() {
    if (!g_Init) {
        g_VersionNum = VersionStr2Num(g_Version);
        g_Init = 1;
    }

    return g_VersionNum;
}
