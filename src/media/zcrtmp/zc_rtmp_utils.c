// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// client
#include "uri-parse.h"
#include "urlcodec.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zc_log.h"
#include "zc_rtmp_utils.h"

bool zc_rtmp_prase_url(const char *inurl, zc_rtmp_url_t *rtmpurl) {
    if (inurl == NULL || rtmpurl == NULL) {
        return false;
    }

    char urltmp[256] = {0};  // rtmp url
    char path[256] = {0};
    char host[128] = {0};
    char *papp = NULL;
    char *pstream = NULL;
    strncpy(urltmp, inurl, sizeof(urltmp) - 1);
    // parse url
    struct uri_t *uri = uri_parse(urltmp, strlen(urltmp));
    if (!uri)
        return false;

    // prase port
    url_decode(uri->path, strlen(uri->path), path, sizeof(path));
    url_decode(uri->host, strlen(uri->host), host, sizeof(host));
    pstream = strrchr(uri->path, '/');
    if (!pstream) {
        LOG_ERROR("rtmp prase error url:%s path:%s", inurl, uri->path);
        uri_free(uri);
        return false;
    }
    papp = uri->path + 1;
    *pstream++ = '\0';
    if (papp[0] == '\0' || pstream[0] == '\0') {
        LOG_ERROR("rtmp prase error url:%s path:%s", inurl, uri->path);
        uri_free(uri);
        return false;
    }

    strncpy(rtmpurl->host, host, sizeof(rtmpurl->host) - 1);
    strncpy(rtmpurl->app, papp, sizeof(rtmpurl->app) - 1);
    strncpy(rtmpurl->stream, pstream, sizeof(rtmpurl->stream) - 1);

    if (uri->port != 0) {
        rtmpurl->port = uri->port;
        snprintf(rtmpurl->rurl, sizeof(rtmpurl->rurl) - 1, "rtmp://%s:%hu/%s", host, rtmpurl->port, rtmpurl->app);
    } else {
        rtmpurl->port = 1935;
        snprintf(rtmpurl->rurl, sizeof(rtmpurl->rurl) - 1, "rtmp://%s:%hu/%s", host, rtmpurl->port, rtmpurl->app);
        snprintf(rtmpurl->rurl, sizeof(rtmpurl->rurl) - 1, "rtmp://%s/%s", host, rtmpurl->app);
    }

    uri_free(uri);
    LOG_TRACE("rtmp prase ok url:%s-> rurl:%s, host:%s, app:%s,stream:%s, port:%lu", inurl, rtmpurl->rurl,
              rtmpurl->host, rtmpurl->app, rtmpurl->stream, rtmpurl->port);
    return true;
}
