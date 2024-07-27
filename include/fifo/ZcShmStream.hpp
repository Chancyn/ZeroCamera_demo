// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from linux kernel(2.6.32) zcfifo.move to userspace, lockfree fifo,
// no locking version just use 1 producer 1 consumer

// shm fifo

#pragma once
#include <pthread.h>
#include <unistd.h>

#include <mutex>

#include "zc_frame.h"

#include "ZcShmFIFO.hpp"

namespace zc {

class CShmStreamW : public CShmFIFO{
 public:
    CShmStreamW(unsigned int size, const char *name, unsigned char chn, stream_puting_cb puting_cb, void *u);
    virtual ~CShmStreamW();

 public:
    // put for hi_venc_stream, 1.first put hdr, 2.stream data PutAppending by m_puting_cb
    unsigned int Put(const unsigned char *buffer, unsigned int len, void *stream);
    // donot lock, puting_cb,callback function need call PutAppending to put framedata append
    unsigned int PutAppending(const unsigned char *buffer, unsigned int len);

 private:
    zc_frame_t *m_latest_idr;    // latest idr
    stream_puting_cb m_puting_cb;
    void *m_u;
    unsigned int m_magic;
    zc_stream_e m_type;
};

class CShmStreamR : public CShmFIFO{
 public:
    CShmStreamR(unsigned int size, const char *name, unsigned char chn);
    virtual ~CShmStreamR();

 public:
    // put for hi_venc_stream, 1.first put hdr, 2.stream data
    unsigned int Get(unsigned char *buffer, unsigned int buflen, unsigned int hdrlen, unsigned int magic);
    bool GetStreamInfo(zc_frame_userinfo_t &info, bool skip2lastest = false);
    void Skip2LatestPos(bool key);

 private:
    unsigned int _getLatestFrameHdr(unsigned char *buffer, unsigned int hdrlen, bool keyflag);
    bool _praseFrameInfo(zc_frame_userinfo_t &info, zc_frame_t *frame);
    bool _getLatestFrameInfo(zc_frame_userinfo_t &info);

    unsigned int m_magic;
    unsigned int m_framemaxlen;
    zc_stream_e m_type;
    zc_frame_t *m_latest_idr;    // latest idr
};
}  // namespace zc
