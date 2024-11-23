// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include "zc_frame.h"

namespace zc {
class CIRtmpPush{
 public:
    CIRtmpPush() {}
    virtual ~CIRtmpPush() {}
 public:
    virtual bool Init(const zc_stream_info_t &info, const char *url) = 0;
    virtual bool UnInit() = 0;
    virtual bool StartCli() = 0;
    virtual bool StopCli() = 0;
};


// mancallback
typedef int (*RtmpPullGetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);
typedef int (*RtmpPullSetInfoCb)(void *ptr, unsigned int chn, zc_stream_info_t *data);

typedef struct {
    RtmpPullGetInfoCb GetInfoCb;
    RtmpPullSetInfoCb SetInfoCb;
    void *MgrContext;
} rtmppull_callback_info_t;

class CIRtmpPull{
 public:
    CIRtmpPull() {}
    virtual ~CIRtmpPull() {}
 public:
    virtual bool Init(rtmppull_callback_info_t *cbinfo, int chn, const char *url) = 0;
    virtual bool UnInit() = 0;
    virtual bool StartCli() = 0;
    virtual bool StopCli() = 0;
};
}
