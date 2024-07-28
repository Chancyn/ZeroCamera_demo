// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstddef>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "NonCopyable.hpp"
#include "mov-buffer.h"
#include "zc_basic_fun.h"
#include "zc_frame.h"
#include "zc_type.h"
#include <string>

#define ZC_FMP4_DEF_PATH "."           // default path
#define ZC_FMP4_PACKING_SUFFIX ".mp4"  // suffix file

#define ZC_SEGMENT_MAXNUM (8)  // segment循环使用数量
// 写movbuf预分配内存大小
#define ZC_CAPSIZE_MIN (500 * 1024)             // capacity 预分配内存块大小
#define ZC_CAPSIZE_DEF ZC_STREAM_MAXFRAME_SIZE  // (1 * 1024 * 1024)  // capacity 预分配内存块大小
#define ZC_CAPSIZE_MAX (4 * 1024 * 1024)        // capacity 4M filebuff,一个segment默认最大大小

// segment 分片最大大小
#define ZC_SEGMENT_MIN_MAXSIZE (4 * 1024 * 1024)   // 1M filebuff,一个segment最大大小（限制最小值）
#define ZC_SEGMENT_DEF_MAXSIZE (6 * 1024 * 1024)   // 4M filebuff,一个segment默认最大大小
#define ZC_SEGMENT_MAX_MAXSIZE (10 * 1024 * 1024)  // 4M filebuff,一个segment默认最大大小

// fmp4 file type
#define ZC_FMP4_TYPE_NAME(auto) (((auto) & 0x1))      // bit0 是否指定名称
#define ZC_FMP4_TYPE_SEGMENT(seg) (((seg)&0x1) << 1)  // bit1 是否自动分片
#define ZC_FMP4_TYPE_APP(app) (((app)&0x7) << 2)      // bit2-bit4 fmp4_app_e
#define ZC_FMP4_TYPE_FLAGS(auto, seg, app) (ZC_FMP4_TYPE_NAME(auto) | ZC_FMP4_TYPE_SEGMENT(seg) | ZC_FMP4_TYPE_APP(app))

namespace zc {
typedef enum {
    fmp4_movio_buf = 0,  // buf
    fmp4_movio_file,     // fmp4file
    fmp4_movio_buffile,  // debug for debug

    fmp4_movio_butt,
} fmp4_movio_e;

typedef enum {
    fmp4_app_none = 0,  // none
    fmp4_app_webs,      // webser
    fmp4_app_hls,       // hls
    fmp4_app_record,    // record

    fmp4_app_butt,
} fmp4_app_e;

typedef struct {
    size_t size;     // 预分配内存大小,每次扩容大小
    size_t maxsize;  // buff最大大小
} zc_movio_bufinfo_t;

typedef struct {
    uint8_t appid;           // appid
    uint8_t segment;         // 文件是否自动分片 1 超过maxsize 分片
    uint32_t segmentnum;     // auto segment 分片数量 0:不循环覆盖,> 1 循环覆盖
    size_t maxsize;          // auto segment下文件最大大小
    const char *path;        // path文件名前缀
    const char *nameprefix;  // path文件名前缀
} zc_movio_fileinfo_t;

typedef struct {
    uint8_t appid;        // appid
    uint8_t segment;      // 文件是否自动分片 1 超过maxsize 分片
    uint32_t segmentnum;  // auto segment 分片数量 0:不循环覆盖,> 1 循环覆盖
    size_t size;          // 预分配内存大小,每次扩容大小
    size_t maxsize;       // buff最大大小
    const char *path;     // path文件名前缀
    const char *nameprefix;
} zc_movio_buffileinfo_t;

typedef union {
    zc_movio_bufinfo_t buf;
    zc_movio_fileinfo_t file;
    zc_movio_buffileinfo_t buffile;
} zc_movio_info_un;

typedef struct {
    fmp4_movio_e type;
    zc_movio_info_un uninfo;
} zc_movio_info_t;

class CMovIo {
 public:
    CMovIo() { memset(&m_io, 0, sizeof(m_io)); }
    virtual ~CMovIo() {}
    const struct mov_buffer_t *GetIoInfo() { return &m_io; }
    virtual int Read(void *data, uint64_t bytes) = 0;
    virtual int Write(const void *data, uint64_t bytes) = 0;
    virtual int Seek(int64_t offset) = 0;
    virtual int64_t Tell() = 0;
    virtual void ResetDataBufPos() = 0;
    virtual const uint8_t *GetDataBufPtr(uint32_t &bytes) = 0;

 protected:
    static int ioRead(void *ptr, void *data, uint64_t bytes);
    static int ioWrite(void *ptr, const void *data, uint64_t bytes);
    static int ioSeek(void *ptr, int64_t offset);
    static int64_t ioTell(void *ptr);

 protected:
    struct mov_buffer_t m_io;
};

class CMovBuf : public CMovIo, public NonCopyable {
 public:
    explicit CMovBuf(const zc_movio_bufinfo_t *info = nullptr);
    virtual ~CMovBuf();

    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();
    virtual void ResetDataBufPos();
    virtual const uint8_t *GetDataBufPtr(uint32_t &bytes);

 private:
    std::vector<uint8_t> m_buf;
    size_t m_bytes;    // readpos
    size_t m_offset;   // writepos
    size_t m_capsize;  // 每次扩容大小 capacity size
    size_t m_maxsize;  // max bytes per mp4 file
};

class CMovFile : public CMovIo, public NonCopyable {
 public:
    explicit CMovFile(const zc_movio_fileinfo_t *info = nullptr);
    virtual ~CMovFile();

    void Close();
    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();
    virtual void ResetDataBufPos() { return; }
    virtual const uint8_t *GetDataBufPtr(uint32_t &bytes) { return nullptr; };

 private:
    std::string m_name;
    std::string m_nameprefix;
    uint8_t m_segmentnum;
    uint32_t m_segmentidx;
    uint32_t m_ftype;
    FILE *m_file;
};

// for debugtest/hls recyle
class CMovBufFile : public CMovIo, public NonCopyable {
 public:
    explicit CMovBufFile(const zc_movio_buffileinfo_t *info = nullptr);
    virtual ~CMovBufFile();
    void Close();
    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();
    virtual void ResetDataBufPos();
    virtual const uint8_t *GetDataBufPtr(uint32_t &bytes);

 private:
    // file
    std::string m_name;
    std::string m_nameprefix;
    uint8_t m_segmentnum;
    uint32_t m_segmentidx;
    uint32_t m_ftype;
    FILE *m_file;

    // buf
    std::vector<uint8_t> m_buf;
    size_t m_bytes;    // readpos
    size_t m_offset;   // writepos
    size_t m_capsize;  // 每次扩容大小 capacity size
    size_t m_maxsize;  // max bytes per mp4 file
};

class CMovIoFac {
 public:
    CMovIoFac() {}
    ~CMovIoFac() {}
    static CMovIo *CMovIoCreate(const zc_movio_info_t &info);
    static CMovIo *CMovIoCreate(fmp4_movio_e type, const zc_movio_info_un *uninfo = nullptr);
};

class CMovReadIo : public CMovIo, public NonCopyable {
 public:
    explicit CMovReadIo(const char *name);
    virtual ~CMovReadIo();
    void Close();
    virtual int Read(void *data, uint64_t bytes);
    virtual int Write(const void *data, uint64_t bytes);
    virtual int Seek(int64_t offset);
    virtual int64_t Tell();
    virtual void ResetDataBufPos() { return; };
    virtual const uint8_t *GetDataBufPtr(uint32_t &bytes) { return nullptr; };

 private:
    FILE *m_file;
    std::string m_name;
};

}  // namespace zc
