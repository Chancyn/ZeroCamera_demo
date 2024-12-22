// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <cstddef>
#include <stdio.h>
#include <string.h>

#include "zc_type.h"
#include "zc_basic_fun.h"
#include "zc_basic_stream.h"
#include "zc_log.h"
#include "zc_macros.h"

#include "Thread.hpp"
#include "ZcType.hpp"
#include "ZcWebMSess.hpp"
#include "ZcFlvSess.hpp"
#include "ZcFmp4Sess.hpp"
#include "ZcTsSess.hpp"
#include "ZcWebServer.hpp"


namespace zc {

// url suffix
static const char *const g_suffixTab[zc_web_msess_type_butt] = {
    "flv",
    "ws.flv",
    "mp4",
    "ws.mp4",
    "ts",
    "ws.ts",
};

IWebMSess *CWebMSessFac::CreateWebMSess(const zc_msess_info_t &info) {
        IWebMSess *sess = nullptr;
        switch (info.type) {
        case zc_web_msess_http_flv:
        case zc_web_msess_ws_flv:
            sess =  new CFlvSess(info.type, info.flvinfo);
            break;
        case zc_web_msess_http_fmp4:
        case zc_web_msess_ws_fmp4:
            sess =  new CFmp4Sess(info.type, info.fmp4info);
            break;
        case zc_web_msess_http_ts:
        case zc_web_msess_ws_ts:
            sess = new CTsSess(info.type, info.tsinfo);
            break;
        default:
            LOG_ERROR("error, type[%d]", info.type);
            break;
        }
        return sess;
}

int zc_get_msess_path(char *dst, unsigned int len, zc_web_msess_type_e mtype, zc_shmstream_e type, unsigned int chn) {
    int ret = 0;
    const char *path = zc_get_livestreampath(type);
    if (!path) {
        return -1;
    }
    ret += snprintf(dst + ret, len - ret, "%s.ch%d.%s", path, chn, g_suffixTab[mtype]);
    // strcat(dst, g_suffixTab[mtype]);
    LOG_TRACE("mtype:%u, type:%u,chn:%u path:%s", mtype, type, chn, dst);
    return ret;
}

// path: live.ch/pushs.ch/pull.ch
int zc_prase_mediasess_path(const char *url, zc_web_msess_type_e *mtype, zc_shmstream_e *type, unsigned int *chn) {
    if (url == NULL || mtype == NULL || type == NULL || chn == NULL) {
        return -1;
    }
    char path[ZC_MAX_PATH] = {0};
    char mainpath[16] = {0};
    char livepath[32] = {0};
    char suffix[16] = {0};
    strncpy(path, url, sizeof(path));
    ZC_UNUSED char *token = nullptr;
    char *ptr = nullptr;
    int channel = 0;
    token = strtok_r(path, " ?", &ptr);

    if (sscanf(path, "/%16[^/]/%32[^.].ch%d.%16s", mainpath, livepath, &channel, suffix) < 4) {
        LOG_WARN("prase token:%s", path);
    }

    if (zc_prase_livestreampath(livepath, type) < 0) {
        return -1;
    }

    zc_web_msess_type_e mtypetmp = zc_web_msess_http_flv;
    for (unsigned int i = 0; i < _SIZEOFTAB(g_suffixTab); i++) {
        if (strncasecmp(suffix, g_suffixTab[i], strlen(g_suffixTab[i])) == 0) {
            mtypetmp = (zc_web_msess_type_e)i;
            break;
        }
    }
    *chn = channel;
    *mtype = mtypetmp;
    LOG_WARN("prase token:%s, mtype:%d, type:%d, chn:%d", path, *mtype, *type, *chn);
    return 0;
}

}  // namespace zc
