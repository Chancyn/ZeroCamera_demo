// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <stdint.h>

#include <memory>
#include <string>

#include "media-source.h"
#include "zc_frame.h"

#include "Thread.hpp"
#include "ZcMediaTrack.hpp"

class CMediaTrack;
namespace zc {
class CLiveSource : public IMediaSource, public Thread {
 public:
    explicit CLiveSource(const zc_stream_info_t &info);
    virtual ~CLiveSource();

 public:
    virtual int Play();
    virtual int Pause();
    virtual int Seek(int64_t pos);
    virtual int SetSpeed(double speed);
    virtual int GetDuration(int64_t &duration) const;
    virtual int GetSDPMedia(std::string &sdp) const;
    virtual int GetRTPInfo(const char *uri, char *rtpinfo, size_t bytes) const;
    virtual int SetTransport(const char *track, std::shared_ptr<IRTPTransport> transport);
    int Init();
    int UnInit();

 private:
    int SendBye();
    virtual int process();
    int _sendProcess();

 private:
    std::string m_sdp;
    int m_status;
    zc_stream_info_t m_info;
    CMediaTrack *m_tracks[ZC_MEDIA_TRACK_BUTT];
};
}  // namespace zc
