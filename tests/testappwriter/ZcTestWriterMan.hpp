// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "ZcModCli.hpp"

namespace zc {
class CTestWriterMan : public CModCli {
 public:
    CTestWriterMan();
    virtual ~CTestWriterMan();

 public:
    bool Init(unsigned int *pCodeTab, unsigned int len);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    bool _unInit();
    static int getStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info);
    int _getStreamInfoCb(unsigned int chn, zc_stream_info_t *info);
    static int setStreamInfoCb(void *ptr, unsigned int chn, zc_stream_info_t *info);
    int _setStreamInfoCb(unsigned int chn, zc_stream_info_t *info);

 private:
    bool m_init;
    bool m_running;
    zc_stream_info_t m_mediainfo[ZC_STREAM_VIDEO_MAX_CHN];
};
}  // namespace zc
