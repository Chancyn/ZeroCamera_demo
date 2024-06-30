#if defined(WITH_FFMPEG)
#include <sys/time.h>
#include <unistd.h>

#include "zc_log.h"
#include "zc_macros.h"

#include "ZcFFEncoder.hpp"

namespace zc {
#define FF_DEFAULT_FPS 30
CFFEncoder::CFFEncoder(const char *camera, const zc_ffcodec_info_t &info) : Thread("CFFEncoder"), m_open(0) {
    static int s_init = 0;
    if (0 == s_init) {
        s_init = 1;
        // avformat_network_init();
        avdevice_register_all();
        avcodec_register_all();
    }

    m_status = 0;
    m_firstpts = 0;

    m_width = info.width;
    m_height = info.height;
    if (info.fps != 0) {
        m_fps = info.fps;
    } else {
        m_fps = FF_DEFAULT_FPS;
    }

    if (ZC_FRAME_ENC_H265 == info.codectype) {
        m_codec_id = AV_CODEC_ID_H265;
    } else if (ZC_FRAME_ENC_H264 == info.codectype) {
        m_codec_id = AV_CODEC_ID_H264;
    } else {
        m_codec_id = AV_CODEC_ID_H264;
    }

    strncpy(m_camname, camera, sizeof(m_camname));
}

CFFEncoder::~CFFEncoder() {
    Close();
}

int CFFEncoder::_closeDev() {
    if (m_ic) {
        avformat_close_input(&m_ic);
        avformat_free_context(m_ic);
        m_ic = nullptr;
    }
    return 0;
}

int CFFEncoder::_closeEncoder() {
    if (m_encctx)
        avcodec_free_context(&m_encctx);
    return 0;
}

int CFFEncoder::_openDev() {
    int ret = 0;
    char video_size[32] = "640x480";
    char framerate[8] = "30";
    char pixel_format[16] = "yuyv422";
    // ctx
    AVFormatContext *fmt_ctx = NULL;
    AVDictionary *opts = NULL;

    snprintf(video_size, sizeof(video_size) - 1, "%ux%u", m_width, m_height);
    snprintf(framerate, sizeof(framerate) - 1, "%u", m_fps);
    // get format
    // ffmpeg -hide_banner -f v4l2 -list_formats all -i /dev/video0
    av_dict_set(&opts, "f", "v4l2", 0);  // 加快探测速度等效 fmt = av_find_input_format("v4l2");
    av_dict_set(&opts, "video_size", video_size, 0);
    av_dict_set(&opts, "framerate", framerate, 0);
    av_dict_set(&opts, "pixel_format", pixel_format, 0);

    LOG_TRACE("input, video_size:%s,framerate:%s,pixel_format:%s", video_size, framerate, pixel_format);
    // open device
    ret = avformat_open_input(&fmt_ctx, m_camname, NULL, &opts);
    if (ret < 0) {
        LOG_ERROR("Failed to open video device, %s,ret:%d", m_camname, ret);
        av_dict_free(&opts);
        return -1;
    }
    av_dict_free(&opts);
    m_ic = fmt_ctx;
    return 0;
}

int CFFEncoder::_openEncoder() {
    int ret = 0;
    AVCodecContext *encctx;
    AVCodec *codec = NULL;
    AVDictionary *opts = NULL;

    // codec = avcodec_find_encoder_by_name("libx264");
    codec = avcodec_find_encoder((AVCodecID)m_codec_id);
    if (!codec) {
        LOG_ERROR("Codec libx264 not found");
        exit(1);
    }

    encctx = avcodec_alloc_context3(codec);
    if (!encctx) {
        LOG_ERROR("Could not allocate video codec context!");
        exit(1);
        return -1;
    }

    // SPS/PPS
    if (m_codec_id == AV_CODEC_ID_H265) {
        encctx->profile = FF_PROFILE_HEVC_MAIN;
    } else {
        encctx->profile = FF_PROFILE_H264_MAIN;  // FF_PROFILE_H264_HIGH_444
    }

    encctx->width = m_width;
    encctx->height = m_height;

    // GOP
    encctx->gop_size = m_fps;
    // encctx->keyint_min = 25;  // option

    // disable b frame
    encctx->max_b_frames = 0;  // option
    encctx->has_b_frames = 0;  // option

    //
    encctx->refs = 3;  // option

    // set encoder YUV420P
    encctx->pix_fmt = AV_PIX_FMT_YUV420P;
    encctx->bit_rate = 600000;  // 600kbps

    // set fps
    encctx->time_base = (AVRational){1, m_fps};  // 帧与帧之间的间隔是time_base
    encctx->framerate = (AVRational){m_fps, 1};  // 帧率，每秒 30帧
    // 设置编码速度
    av_dict_set(&opts, "preset", "fast", 0);
    ret = avcodec_open2(encctx, codec, &opts);
    if (ret < 0) {
        LOG_ERROR("Could not open codec: %d!", ret);
        av_dict_free(&opts);
        exit(1);
        return -1;
    }
    m_encctx = encctx;
    av_dict_free(&opts);

    return 0;
}

bool CFFEncoder::Open() {
    if (m_open) {
        return false;
    }

    if (0 != _openDev()) {
        LOG_ERROR("open error: ");
        goto _err;
    }

    if (0 != _openEncoder()) {
        LOG_ERROR("open Encoder error: ");
        goto _err;
    }

    Start();
    m_open = 1;
    LOG_TRACE("open:%s ok ", m_camname);

    return true;
_err:
    _closeEncoder();
    _closeDev();
    LOG_ERROR("Open:%s error", m_camname);
    return false;
}

bool CFFEncoder::Close() {
    if (!m_open) {
        return false;
    }

    Stop();
    _closeEncoder();
    _closeDev();
    m_open = 0;
    LOG_ERROR("Close:%s ok", m_camname);
    return true;
}

int CFFEncoder::_encode(AVFrame *frame, AVPacket *pkt) {
    int ret;

    /* send the frame to the encoder */
    // if (frame)
    //     LOG_TRACE("Send frame %lld", frame->pts);

    ret = avcodec_send_frame(m_encctx, frame);
    if (ret < 0) {
        LOG_ERROR("Error sending a frame for encoding");
        ZC_ASSERT(0);
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(m_encctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            LOG_ERROR("Error during encoding");
            ZC_ASSERT(0);
            return ret;
        }
        struct timespec tp;
        clock_gettime(CLOCK_MONOTONIC, &tp);
        uint64_t clock = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);

        if (m_firstpts == 0) {
            m_firstpts = clock;
            LOG_DEBUG("###debug first pts:%lld, cos:%lld ", m_firstpts, m_firstpts - m_startutc);
        }

        // pkt->pts = av_rescale_q(frame->pts, m_encctx->time_base, {1, 1000});        // time ms
        // pkt->pts = av_rescale_q(frame->pts, m_encctx->time_base, {1, AV_TIME_BASE});        // time us

        pkt->dts = pkt->pts = clock;
        if (pkt->flags) {
            LOG_DEBUG("flag:%d,len:%05d,pts:%lld,dts:%lld,clock:%llu", pkt->flags, pkt->size, pkt->pts, pkt->dts,
                      clock);
        }
#if ZC_FFENCODER_DEBUG
        fwrite(pkt->data, 1, pkt->size, m_debugfp);
        fflush(m_debugfp);
#endif
        av_packet_unref(pkt);
    }

    return 0;
}

void CFFEncoder::_yuyv422ToYuv420p(AVFrame *frame, AVPacket *pkt) {
    int i = 0;
    int yuv422_length = m_width * m_height * 2;
    int y_index = 0;
    // copy all y
    for (i = 0; i < yuv422_length; i += 2) {
        frame->data[0][y_index] = pkt->data[i];
        y_index++;
    }

    // copy u and v
    int line_start = 0;
    int is_u = 1;
    int u_index = 0;
    int v_index = 0;
    // copy u, v per line. skip a line once
    for (i = 0; i < m_height; i += 2) {
        // line i offset
        line_start = i * m_width * 2;
        for (int j = line_start + 1; j < line_start + m_width * 2; j += 4) {
            frame->data[1][u_index] = pkt->data[j];
            u_index++;
            frame->data[2][v_index] = pkt->data[j + 2];
            v_index++;
        }
    }
}

int CFFEncoder::_encodecProcess() {
    int ret = 0;
    int i = 0;
    AVPacket *inpkt = av_packet_alloc();
    av_init_packet(inpkt);
    AVPacket *pkt = av_packet_alloc();
    av_init_packet(pkt);
    AVFrame *frame = av_frame_alloc();
    frame->format = m_encctx->pix_fmt;
    frame->width = m_encctx->width;
    frame->height = m_encctx->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        LOG_ERROR("Could not allocate the video frame data");
        exit(1);
    }

    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    uint64_t clock = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
    m_startutc = clock;
#if ZC_FFENCODER_DEBUG
    FILE *fp;
    char filename[128];
    snprintf(filename, sizeof(filename) - 1, "%s.%s", ZC_FFENCODER_DEBUG_PATH, "h264");
    fp = fopen(filename, "wb");
    if (!fp) {
        LOG_ERROR("Could not open %s", filename);
    } else {
        m_debugfp = fp;
    }
#endif

    while (State() == Running) {
        if ((ret = av_read_frame(m_ic, inpkt)) < 0) {
            LOG_ERROR("read_frame error, ret:%d", ret);
            goto _err;
        }

        _yuyv422ToYuv420p(frame, inpkt);
        frame->pts = i++;

        if ((ret = _encode(frame, pkt)) < 0) {
            LOG_ERROR("_encode error, ret:%d", ret);
            av_packet_unref(inpkt);
            goto _err;
        }

        av_packet_unref(inpkt);
    }

    _encode(NULL, pkt);
_err:
    av_frame_free(&frame);
    av_packet_free(&pkt);
    av_packet_free(&inpkt);
#if ZC_FFENCODER_DEBUG
    if (m_debugfp) {
        fclose(m_debugfp);
        m_debugfp = nullptr;
    }
#endif
    LOG_TRACE("encode process ret:%d", ret);
    return ret;
}

int CFFEncoder::Pause() {
    m_status = 2;
    return 0;
}

int CFFEncoder::process() {
    LOG_WARN("process into");
    int ret = 0;
    int64_t dts = 0;

    while (State() == Running) {
        ret = _encodecProcess();
        if (ret == -1) {
            LOG_WARN("process file EOF exit");
            ret = 0;
            goto _err;
        }
        usleep(10 * 1000);
    }
_err:

    LOG_WARN("process exit");
    return ret;
}
}  // namespace zc

#endif
