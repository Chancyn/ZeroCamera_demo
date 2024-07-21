// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>

#include "zc_basic_fun.h"
#include "zc_basic_stream.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcType.hpp"
#include "ZcWebMSess.hpp"
#include "ZcWebServer.hpp"
#include "zc_type.h"

namespace zc {

//
static const char *const g_suffixTab[zc_web_msess_type_butt] = {
    ".flv",
    ".ws.flv",
    ".mp4",
    ".ws.mp4",
};

int zc_get_msess_path(char *dst, unsigned int len, zc_web_msess_type_e mtype, zc_shmstream_e type, unsigned int chn) {
    int ret = zc_get_livestreampath(dst, len, type, chn);
    if (ret < 0) {
        return -1;
    }
    ret += snprintf(dst + ret, len - ret, "%s", g_suffixTab[mtype]);
    // strcat(dst, g_suffixTab[mtype]);
    LOG_TRACE("mtype:%u, type:%u,chn:%u path:%s", mtype, type, chn, dst);
    return ret;
}

// path: live.ch/pushs.ch/pull.ch
int zc_prase_mediasess_path(const char *url, zc_web_msess_type_e *mtype, zc_shmstream_e *type, unsigned int *chn) {
    if (url == NULL || mtype== NULL || type == NULL || chn == NULL) {
        return -1;
    }
    char path[ZC_MAX_PATH] = {0};

    const char *tmp = strstr(url, "/live/");
    if (!tmp) {
        return -1;
    }
    tmp += strlen("/live/");

    strncpy(path, tmp, sizeof(path));
    char *token = nullptr;
    char *ptr = nullptr;
    token = strtok_r(path, " ?", &ptr);
    if (zc_prase_livestreampath(path, type, chn) < 0) {
        return -1;
    }
    zc_web_msess_type_e mtypetmp = zc_web_msess_http_flv;
    for (unsigned int i = 0; i < _SIZEOFTAB(g_suffixTab); i++) {
        if (strstr(path, g_suffixTab[i]) != nullptr) {
            mtypetmp = (zc_web_msess_type_e)i;
            break;
        }
    }
    *mtype = mtypetmp;
    LOG_WARN("prase token:%s, mtype:%d, type:%d, chn:%d", path, *mtype, *type, *chn);
    return 0;
}

}  // namespace zc
