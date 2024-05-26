// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <asm-generic/errno.h>
#include <mutex>
#include <stdio.h>

#include <memory>

#include "Thread.hpp"
#include "rtp-payload.h"
#include "rtp-profile.h"
#include "rtp.h"
#include "sys/path.h"
#include "sys/system.h"

#include "ZcType.hpp"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_type.h"

#include "Epoll.hpp"
#include "ZcLiveSource.hpp"
#include "ZcMediaTrack.hpp"
#include "ZcMediaTrackFac.hpp"

extern "C" uint32_t rtp_ssrc(void);

#define VIDEO_BANDWIDTH (4 * 1024)
#define AUDIO_BANDWIDTH (4 * 1024)  // bandwidth

namespace zc {
// CLiveSource::CLiveSource() :m_count(MEDIA_TRACK_BUTT){}
CLiveSource::CLiveSource() : Thread("LiveSource"), m_status(0), m_count(MEDIA_TRACK_META) {
    for (int i = 0; i < MEDIA_TRACK_BUTT; i++) {
        m_tracks[i] = nullptr;
    }
    Init();
}

CLiveSource::~CLiveSource() {
    UnInit();
}

int CLiveSource::SetTransport(const char *track, std::shared_ptr<IRTPTransport> transport) {
    int t = atoi(track + 5 /*track*/);
    if (t < 0 || t > m_count) {
        return -1;
    }

    CMediaTrack *mtrack = m_tracks[t];
    mtrack->SetTransport(transport);
    return 0;
}

int CLiveSource::Play() {
    m_status = 1;

    return 0;
}

int CLiveSource::Pause() {
    m_status = 2;
    return 0;
}

int CLiveSource::Seek(int64_t pos) {
    return 0;
}

int CLiveSource::SetSpeed(double speed) {
    return 0;
}

int CLiveSource::GetDuration(int64_t &duration) const {
    return -1;
}

int CLiveSource::UnInit() {
    Stop();
    for (int i = 0; i < MEDIA_TRACK_BUTT; i++) {
        ZC_SAFE_DELETE(m_tracks[i]);
    }

    return 0;
}

int CLiveSource::Init() {
    CMediaTrackFac fac;

    for (int i = 0; i < m_count; i++) {
        CMediaTrack *mtrack = nullptr;
        if (i == MEDIA_TRACK_VIDEO) {
            mtrack = fac.CreateMediaTrack(MEDIA_CODE_H264);
        } else if (i == MEDIA_TRACK_AUDIO) {
            mtrack = fac.CreateMediaTrack(MEDIA_CODE_AAC);
        } else if (i == MEDIA_TRACK_META) {
            mtrack = fac.CreateMediaTrack(MEDIA_CODE_METADATA);
        }

        if (!mtrack) {
            continue;
        }

        if (!mtrack->Init()) {
            delete mtrack;
            continue;
        }

        std::string sdp;
        mtrack->GetSDPMedia(sdp);
        m_sdp.append(sdp);
        m_tracks[i] = mtrack;
    }

    // start send thread
    Start();

    LOG_WARN("sdp[%s]", m_sdp.c_str());
    return 0;
}

int CLiveSource::GetSDPMedia(std::string &sdp) const {
#if 1
    sdp = m_sdp;
#else
    CMediaTrack *mtrack = nullptr;
    for (int i = 0; i < m_count; i++) {
        mtrack = m_tracks[i];
        if (!mtrack) {
            continue;
        }
        std::string sdptrack;
        mtrack->GetSDPMedia(sdptrack);
        sdp.append(sdptrack);
    }
#endif
    return 0;
}

int CLiveSource::GetRTPInfo(const char *uri, char *rtpinfo, size_t bytes) const {
    int n = 0;

    // RTP-Info: url=rtsp://foo.com/bar.avi/streamid=0;seq=45102,
    //           url=rtsp://foo.com/bar.avi/streamid=1;seq=30211
    CMediaTrack *mtrack = nullptr;
    for (int i = 0; i < m_count; i++) {
        mtrack = m_tracks[i];
        if (!mtrack) {
            continue;
        }

        n = strlen(rtpinfo);
        if (n > 0)
            rtpinfo[n++] = ',';

        mtrack->GetRTPInfo(uri, rtpinfo + n, bytes - n);
    }
    return 0;
}

int CLiveSource::_sendProcess() {
    LOG_WARN("process into\n");
    CEpoll ep{100};     // set timeout 100ms,for rtspsource thread exit
    int ret = 0;
#if 1
    if (!ep.Create()) {
        LOG_ERROR("epoll create error");
        return -1;
    }

    for (int i = 0; i < m_count; i++) {
        int evfd = -1;
        if (m_tracks[i] && (evfd = m_tracks[i]->GetEvFd()) > 0) {
            LOG_WARN("epoll add i[%d], fd[%d] ptr[%p]", i, evfd, m_tracks[i]);
            ep.Add(evfd, EPOLLIN | EPOLLET, m_tracks[i]);
        }
    }

    while (State() == Running /*&&  i < loopcnt*/) {
        ret = ep.Wait();
        if (ret == -1) {
            LOG_ERROR("epoll wait error");
            return -1;
        } else if (ret > 0) {
            for (int i = 0; i < ret; i++) {
                if (ep[i].events & EPOLLIN) {
                    CMediaTrack *tack = reinterpret_cast<CMediaTrack *>(ep[i].data.ptr);
                    // LOG_TRACE("epoll wait ok ret[%d], tack[%d]", ret, tack);
                    tack->GetData2Send();
                }
            }
        }
    }
#endif
    LOG_WARN("process exit\n");
    return -1;
}

int CLiveSource::process() {
    LOG_WARN("process into\n");
    int ret = 0;
    while (State() == Running /*&&  i < loopcnt*/) {
        if (m_status == 1) {
            ret = _sendProcess();
            if (ret < 0) {
                LOG_WARN("process exit\n");
                goto _err;
            }
        }
    }
_err:

    LOG_WARN("process exit\n");
    return -1;
}

}  // namespace zc
