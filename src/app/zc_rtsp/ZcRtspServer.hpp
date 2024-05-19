// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "rtsp-server-internal.h"
#include "rtsp-server.h"

#include "rtsp/ZcModRtsp.hpp"

namespace zc {
class CRtspServer {
 public:
    CRtspServer();
    virtual ~CRtspServer();

 public:
    bool Init();
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    bool _init();
    bool _unInit();
    int rtsp_uri_parse(const char* uri, std::string& path);



    static int rtsp_ondescribe(void *ptr, rtsp_server_t *rtsp, const char *uri);
    int _ondescribe(void *ptr, rtsp_server_t *rtsp, const char *uri);
    static int rtsp_onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                            const struct rtsp_header_transport_t transports[], size_t num);
    int _onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                 const struct rtsp_header_transport_t transports[], size_t num);
    static int rtsp_onplay(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                           const double *scale);
    int _onplay(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                const double *scale);
    static int rtsp_onpause(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt);
    int _onpause(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt);
    static int rtsp_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session);
    int _onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session);
    static int rtsp_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len);
    int _onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp, int len);
    static int rtsp_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                             const double *scale);
    int _onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                  const double *scale);
    static int rtsp_onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri);
    int _onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri);
    static int rtsp_ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                   const void *content, int bytes);
    int _ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const void *content,
                        int bytes);
    static int rtsp_onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                   const void *content, int bytes);
    int _onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const void *content,
                        int bytes);
    static int rtsp_onclose(void *ptr2);
    int _onclose(void *ptr2);

    static void rtsp_onerror(void *ptr, rtsp_server_t *rtsp, int code);
    void _onerror(void *ptr, rtsp_server_t *rtsp, int code);

 private:
    bool m_init;
    int m_running;
    void *m_phandle;  // handle
    void *m_psvr;     // server
};
}  // namespace zc
