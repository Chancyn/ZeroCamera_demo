// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include <string.h>
#include <vector>

#include "NonCopyable.hpp"
#include "fmp4-writer.h"
#include "mov-buffer.h"
#include "zc_basic_fun.h"
#include "zc_frame.h"
#include "zc_media_track.h"
#include "zc_type.h"

#include "Thread.hpp"
#include "ZcShmStream.hpp"

#define ZC_FMP4_DEF_PATH "."           // default path
#define ZC_FMP4_PACKING_SUFFIX "mp4"  // suffix file
// YY-YY-MMDDThhmmss_randidx_def
#define ZC_FMP4_FRAME_HDR 128  // debug

namespace zc {
typedef enum {
    fmp4_movio_buf = 0,  // buf
    fmp4_movio_file,     // fmp4file
    fmp4_movio_buffile,  // debug for debug

    fmp4_movio_butt,
} fmp4_movio_e;

typedef int (*OnFmp4PacketCb)(void *param, int type, const void *data, size_t bytes, uint32_t timestamp);

typedef struct {
    fmp4_movio_e type;  // type
    const char *name;   // file filename
    zc_stream_info_t streaminfo;
    OnFmp4PacketCb onfmp4packetcb;
    void *Context;
} zc_fmp4muxer_info_t;

class CMovIo {
 public:
    CMovIo() { memset(&m_io, 0, sizeof(m_io)); }
    virtual ~CMovIo() {}
    const struct mov_buffer_t *GetIoInfo() { return &m_io; }
    virtual int Read(void *data, uint64_t bytes) = 0;
    virtual int Write(const void *data, uint64_t bytes) = 0;
    virtual int Seek(int64_t offset) = 0;
    virtual int64_t Tell() = 0;
 protected:
    static int ioRead(void *ptr, void *data, uint64_t bytes);
    static int ioWrite(void *ptr, const void *data, uint64_t bytes);
    static int ioSeek(void *ptr, int64_t offset);
    static int64_t ioTell(void *ptr);
 protected:
    struct mov_buffer_t m_io;
};

class CMovBuf : public CMovIo, public NonCopyable{
 public:
    CMovBuf();
    virtual ~CMovBuf();

    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();

 private:
    uint8_t *m_buf;
    size_t m_bytes;
    size_t m_offset;
    size_t m_capacity;
    size_t m_maxsize;  // max bytes per mp4 file
};

class CMovFile : public CMovIo, public NonCopyable {
 public:
    explicit CMovFile(const char *name);
    virtual ~CMovFile();

    void Close();
    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();

 private:
    char m_name[ZC_MAX_PATH];
    FILE *m_file;
};

class CMovBufFile : public CMovIo, public NonCopyable {
 public:
    explicit CMovBufFile(const char *name);
    virtual ~CMovBufFile();

    void Close();
    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();

 private:
    char m_name[ZC_MAX_PATH];
    FILE *m_file;
    uint8_t *m_buf;
    size_t m_bytes;
    size_t m_offset;
    size_t m_capacity;
    size_t m_maxsize;  // max bytes per mp4 file
};

class CMovIoFac {
 public:
    CMovIoFac() {}
    ~CMovIoFac() {}
    static CMovIo *CMovIoCreate(fmp4_movio_e type, const char *name);
};

class CFmp4Muxer : protected Thread {
    enum fmp4_status_e {
        fmp4_status_err = -1,  // error
        fmp4_status_init = 0,  // init
        fmp4_status_run,       // running
    };

 public:
    CFmp4Muxer();
    virtual ~CFmp4Muxer();

 public:
    bool Create(const zc_fmp4muxer_info_t &info);
    bool Destroy();
    bool Start();
    bool Stop();
    fmp4_status_e GetStatus() { return m_status; }

 private:
    bool destroyStream();
    bool createStream();
    int _write2Fmp4(zc_frame_t *pframe);
    int _getDate2Write2Fmp4(CShmStreamR *stream);
    int _packetProcess();
    virtual int process();

 private:
    bool m_Idr;
    fmp4_status_e m_status;
    fmp4_movio_e m_type;
    ZC_U64 m_pts;
    ZC_U64 m_apts;  // audio pts
    unsigned char m_framebuf[ZC_STREAM_MAXFRAME_SIZE];
    unsigned char m_framemp4buf[ZC_STREAM_MAXFRAME_SIZE];  // TODO + hdr
    zc_fmp4muxer_info_t m_info;
    CMovIo *m_movio;
    struct fmp4_writer_t *m_fmp4;
    int m_trackid[ZC_MEDIA_TRACK_BUTT];
    std::vector<CShmStreamR *> m_vector;
};

}  // namespace zc
