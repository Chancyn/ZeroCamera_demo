#pragma once

#if defined(WITH_FFMPEG)
#include <stdint.h>
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
}

#include "zc_frame.h"

#include <memory>

#include "Thread.hpp"

#if ZC_DEBUG
#define ZC_FFENCODER_DEBUG 1
#define ZC_FFENCODER_DEBUG_PATH "./ffencode_debug"  // debug save file
#endif
namespace zc {
typedef struct _zc_ffcodec_info_ {
    unsigned int codectype;
    unsigned int fps;
    unsigned int width;
    unsigned int height;
} zc_ffcodec_info_t;

class CFFEncoder : public Thread {
 public:
    CFFEncoder(const char *camera, const zc_ffcodec_info_t &info);
    virtual ~CFFEncoder();

 public:
    int Play();
    int Pause();

    bool Open();
    bool Close();

 private:
    int _open();

    virtual int process();
    int _openDev();
    int _closeDev();
    int _openEncoder();
    int _closeEncoder();
    int _readFrame();
    int _encodecProcess();
    int _encode(AVFrame *frame, AVPacket *pkt);
    void _yuyv422ToYuv420p(AVFrame *frame, AVPacket *pkt);

 private:
    int m_open;
    int64_t m_startutc;
    int64_t m_firstpts;

    int m_status;
    int64_t m_pos;
    unsigned int m_fps;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_codec_id;
    AVFormatContext *m_ic;
    AVCodecContext *m_encctx;
    int m_count;
    char m_camname[64];
#if ZC_FFENCODER_DEBUG
    FILE *m_debugfp;
#endif
};
}  // namespace zc
#endif
