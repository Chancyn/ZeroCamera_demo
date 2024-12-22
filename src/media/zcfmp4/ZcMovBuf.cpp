// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
// #include <sys/syslog.h>
#include <sys/types.h>
#include <unistd.h>

#include "fmp4-writer.h"
#include "mov-format.h"
#include "mp4-writer.h"
#include "mpeg4-aac.h"
#include "mpeg4-avc.h"
#include "mpeg4-hevc.h"
#include "zc_log.h"

#include "ZcMovBuf.hpp"
#include "ZcType.hpp"
#include "zc_macros.h"

namespace zc {
#define ZC_FMP4MUXER_DEBUG_DUMP 1

#if defined(_WIN32) || defined(_WIN64)
#define fseek64 _fseeki64
#define ftell64 _ftelli64
#elif defined(__ANDROID__)
#define fseek64 fseek
#define ftell64 ftell
#elif defined(OS_LINUX)
#define fseek64 fseeko64
#define ftell64 ftello64
#else
#define fseek64 fseek
#define ftell64 ftell
#endif

int CMovIo::ioRead(void *ptr, void *data, uint64_t bytes) {
    CMovIo *self = reinterpret_cast<CMovIo *>(ptr);
    return self->Read(data, bytes);
}

int CMovIo::ioWrite(void *ptr, const void *data, uint64_t bytes) {
    CMovIo *self = reinterpret_cast<CMovIo *>(ptr);
    return self->Write(data, bytes);
}

int CMovIo::ioSeek(void *ptr, int64_t offset) {
    CMovIo *self = reinterpret_cast<CMovIo *>(ptr);
    return self->Seek(offset);
}

int64_t CMovIo::ioTell(void *ptr) {
    CMovIo *self = reinterpret_cast<CMovIo *>(ptr);
    return self->Tell();
}

CMovBuf::CMovBuf(const zc_movio_bufinfo_t *info /*= nullptr*/) {
    zc_movio_bufinfo_t definfo = {
        .size = ZC_CAPSIZE_DEF,
        .maxsize = ZC_SEGMENT_DEF_MAXSIZE,
    };

    if (!info) {
        info = &definfo;
    }

    if (info->size != 0) {
        m_capsize = info->size > ZC_CAPSIZE_MAX ? ZC_CAPSIZE_MAX : info->size;
        m_capsize = m_capsize < ZC_CAPSIZE_MIN ? ZC_CAPSIZE_MIN : m_capsize;
    } else {
        m_capsize = ZC_CAPSIZE_DEF;
    }

    if (info->maxsize != 0) {
        m_maxsize = info->maxsize > ZC_SEGMENT_MAX_MAXSIZE ? ZC_SEGMENT_MAX_MAXSIZE : info->maxsize;
        m_maxsize = m_maxsize < ZC_SEGMENT_MIN_MAXSIZE ? ZC_SEGMENT_MIN_MAXSIZE : m_maxsize;
    } else {
        m_maxsize = ZC_SEGMENT_DEF_MAXSIZE;
    }
    //m_buf.reserve(m_capsize);
    m_buf.resize(m_capsize);
    LOG_INFO("###debug m_capsize:%zu, capacity:%zu, m_maxsize:%zu", m_capsize, m_buf.capacity(), m_maxsize);

    m_bytes = 0;
    m_offset = 0;
    m_io.read = ioRead;
    m_io.write = ioWrite;
    m_io.seek = ioSeek;
    m_io.tell = ioTell;
}

CMovBuf::~CMovBuf() {}

int CMovBuf::Read(void *data, uint64_t bytes) {
    if (m_offset + bytes > m_bytes)
        return E2BIG;

    memcpy(data, &m_buf[m_offset], (uint64_t)bytes);
    m_offset += bytes;

    return 0;
}

void CMovBuf::ResetDataBufPos() {
    m_offset = 0;
    m_bytes = 0;
    return;
}

const uint8_t *CMovBuf::GetDataBufPtr(uint32_t &bytes) {
    const uint8_t *ptr = nullptr;
#if ZC_FMP4MUXER_DEBUG_DUMP
    if (bytes)
        LOG_TRACE("getdatabuf ptr: off:%zu, byte:%zu", m_offset, m_bytes);
#endif
    if (m_offset > m_bytes) {
        bytes = m_offset - m_bytes;
        ptr = &m_buf[m_bytes];
        m_bytes = m_offset;
    }

    return ptr;
}

int CMovBuf::Write(const void *data, uint64_t bytes) {
    if (m_offset + bytes > m_maxsize) {
        LOG_ERROR("###debug,write off:%zu+bytes:%llu = %zu > %zu maxsize", m_offset, bytes, m_offset + bytes, m_maxsize);
        return -E2BIG;
    }

    if (m_offset + bytes > m_buf.capacity()) {
        size_t needsize;
        needsize = m_offset + bytes + m_capsize;
        needsize = needsize > m_maxsize ? m_maxsize : needsize;
        LOG_WARN("###debug capacity off:%zu, byte:%llu, capacity:%zu; needsize:%zu", m_offset, bytes, m_buf.capacity(),
                 needsize);
        // m_buf.reserve(needsize);
        m_buf.resize(needsize);
    }

    memcpy(&m_buf[m_offset], data, bytes);
    m_offset += bytes;

    return 0;
}

int CMovBuf::Seek(int64_t offset) {
    if ((offset >= 0 ? offset : -offset) >= m_maxsize)
        return -E2BIG;

    ZC_ASSERT((size_t)offset <= m_buf.capacity());

    m_offset = (size_t)(offset >= 0 ? offset : m_maxsize + offset);
    return 0;
}

int64_t CMovBuf::Tell() {
    return (int64_t)m_offset;
}

static void generateSegmentName(std::string &name, const std::string &prefix, uint32_t idx) {
    name = prefix;
    char idxstr[8];
    snprintf(idxstr, sizeof(idxstr), "_%0u", idx);
    name.append(idxstr);
    name.append(ZC_FMP4_PACKING_SUFFIX);
    return;
}

CMovFile::CMovFile(const zc_movio_fileinfo_t *info /*= nullptr*/) : m_segmentidx(0) {
    zc_movio_fileinfo_t definfo{};
    if (!info) {
        info = &definfo;
    }
    const char *path = info->path;
    if (path == nullptr || path[0] == '\0') {
        path = ZC_FMP4_DEF_PATH;
    }

    m_nameprefix = path;
    if (m_nameprefix[m_nameprefix.length() - 1] != '/') {
        m_nameprefix.append("/");
    }

    const char *namepre = info->nameprefix;
    char file[128];
    ZC_U32 autoname = 0;
    if (namepre == nullptr || namepre[0] == '\0') {
        namepre = GenerateFileName(file, sizeof(file));
        LOG_WARN("name null; generate filename:%s", namepre);
        autoname = 1;
    }
    m_nameprefix.append(namepre);

    m_ftype = ZC_FMP4_TYPE_FLAGS(autoname, info->segment, info->appid);
    char typestr[8];
    snprintf(typestr, sizeof(typestr), "_%04x", m_ftype);
    m_nameprefix.append(typestr);

    // name
    generateSegmentName(m_name, m_nameprefix, m_segmentidx);

    LOG_WARN("###debug prefix:%s, open:%s", m_nameprefix.c_str(), m_name.c_str());
    m_file = fopen(m_name.c_str(), "wb+");
    if (m_file) {
        m_io.read = ioRead;
        m_io.write = ioWrite;
        m_io.seek = ioSeek;
        m_io.tell = ioTell;
    } else {
        LOG_ERROR("error open:%s", m_name.c_str());
    }
}

CMovFile::~CMovFile() {
    Close();
}

void CMovFile::Close() {
    if (m_file) {
        fsync(fileno(m_file));
        fclose(m_file);
        m_file = nullptr;
        LOG_TRACE("close:%s", m_name.c_str());
    }
    return;
}

int CMovFile::Read(void *data, uint64_t bytes) {
    if (bytes == fread(data, 1, bytes, m_file))
        return 0;
    return 0 != ferror(m_file) ? ferror(m_file) : -1 /*EOF*/;
}

int CMovFile::Write(const void *data, uint64_t bytes) {
    return bytes == fwrite(data, 1, bytes, m_file) ? 0 : ferror(m_file);
}

int CMovFile::Seek(int64_t offset) {
    return fseek64(m_file, offset, offset >= 0 ? SEEK_SET : SEEK_END);
}

int64_t CMovFile::Tell() {
    return ftell64(m_file);
}

CMovBufFile::CMovBufFile(const zc_movio_buffileinfo_t *info /*= nullptr*/) {
    zc_movio_buffileinfo_t definfo = {
        .size = ZC_CAPSIZE_DEF,
        .maxsize = ZC_SEGMENT_DEF_MAXSIZE,
    };

    if (!info) {
        info = &definfo;
    }

    if (info->size != 0) {
        m_capsize = info->size > ZC_CAPSIZE_MAX ? ZC_CAPSIZE_MAX : info->size;
        m_capsize = m_capsize < ZC_CAPSIZE_MIN ? ZC_CAPSIZE_MIN : m_capsize;
    } else {
        m_capsize = ZC_CAPSIZE_DEF;
    }

    if (info->maxsize != 0) {
        m_maxsize = info->maxsize > ZC_SEGMENT_MAX_MAXSIZE ? ZC_SEGMENT_MAX_MAXSIZE : info->maxsize;
        m_maxsize = m_maxsize < ZC_SEGMENT_MIN_MAXSIZE ? ZC_SEGMENT_MIN_MAXSIZE : m_maxsize;
    } else {
        m_maxsize = ZC_SEGMENT_DEF_MAXSIZE;
    }

    // m_buf.reserve(m_capsize);
    m_buf.resize(m_capsize);
    m_bytes = 0;
    m_offset = 0;
    LOG_INFO("###debug m_capsize:%zu, capacity:%zu, m_maxsize:%zu", m_capsize, m_buf.capacity(), m_maxsize);
    // file
    const char *path = info->path;
    if (path == nullptr || path[0] == '\0') {
        path = ZC_FMP4_DEF_PATH;
    }

    m_nameprefix = path;
    if (m_nameprefix[m_nameprefix.length() - 1] != '/') {
        m_nameprefix.append("/");
    }

    const char *namepre = info->nameprefix;
    char file[128];
    ZC_U32 autoname = 0;
    if (namepre == nullptr || namepre[0] == '\0') {
        namepre = GenerateFileName(file, sizeof(file));
        LOG_WARN("name null; generate filename:%s", namepre);
        autoname = 1;
    }
    m_nameprefix.append(namepre);

    m_ftype = ZC_FMP4_TYPE_FLAGS(autoname, info->segment, info->appid);
    char typestr[8];
    snprintf(typestr, sizeof(typestr), "_%04x", m_ftype);
    m_nameprefix.append(typestr);

    // name
    generateSegmentName(m_name, m_nameprefix, m_segmentidx);

    LOG_WARN("###debug prefix:%s, open:%s", m_nameprefix.c_str(), m_name.c_str());
    m_file = fopen(m_name.c_str(), "wb+");
    if (m_file) {
        m_io.read = ioRead;
        m_io.write = ioWrite;
        m_io.seek = ioSeek;
        m_io.tell = ioTell;
    } else {
        LOG_ERROR("error open:%s", m_name.c_str());
    }
}

CMovBufFile::~CMovBufFile() {
    Close();
}

void CMovBufFile::Close() {
    if (m_file) {
        fsync(fileno(m_file));
        fclose(m_file);
        m_file = nullptr;
        LOG_TRACE("close:%s", m_name.c_str());
    }
    return;
}

int CMovBufFile::Read(void *data, uint64_t bytes) {
    if (m_offset + bytes > m_bytes)
        return E2BIG;
    // LOG_TRACE("read, offset:%zu, size:%llu", m_offset, bytes);
    memcpy(data, &m_buf[m_offset], (uint64_t)bytes);
    m_offset += bytes;
    size_t ret = 0;
    if (bytes == (size_t)(ret = fread(data, 1, bytes, m_file)))
        return 0;
    int fret = ferror(m_file);
    if (fret != 0) {
        LOG_ERROR("###debug read ret:%d, fret:%d, offset:%u, bytes:%u", ret, fret, m_offset, bytes);
        return fret;
    } else {
        LOG_ERROR("###debug read EOF ret:%d, fret=%d, offset:%u, bytes:%u", ret, fret, m_offset, bytes);
        return -1;
    }

    // if (bytes  == fread(data, 1, bytes, m_file))
    //       return 0;
    // return 0 != ferror(m_file) ? ferror(m_file) : -1 /*EOF*/;
}

void CMovBufFile::ResetDataBufPos() {
    m_offset = 0;
    m_bytes = 0;
    return;
}

const uint8_t *CMovBufFile::GetDataBufPtr(uint32_t &bytes) {
    const uint8_t *ptr = nullptr;

#if ZC_FMP4MUXER_DEBUG_DUMP
    if (bytes)
        LOG_TRACE("getdatabuf ptr: off:%zu, byte:%zu", m_offset, m_bytes);
#endif

    if (m_offset > m_bytes) {
        bytes = m_offset - m_bytes;
        ptr = &m_buf[m_bytes];
        m_bytes = m_offset;
    }

    return ptr;
}

int CMovBufFile::Write(const void *data, uint64_t bytes) {
    if (m_offset + bytes > m_maxsize) {
        LOG_ERROR("###debug,write off:%zu+bytes:%llu = %zu > %zu maxsize", m_offset, bytes, m_offset + bytes, m_maxsize);
        return -E2BIG;
    }

    // LOG_WARN("write,cap:%zu,offset:%zu,max:%zu, size:%llu", m_buf.capacity(), m_offset, m_maxsize, bytes);
    if (m_offset + bytes > m_buf.capacity()) {
        size_t needsize;
        needsize = m_offset + bytes + m_capsize;
        needsize = needsize > m_maxsize ? m_maxsize : needsize;
        LOG_WARN("###debug capacity off:%zu, byte:%llu, capacity:%zu; needsize:%zu", m_offset, bytes, m_buf.capacity(),
                 needsize);
        // m_buf.reserve(needsize);
        m_buf.resize(needsize);

    }

    memcpy(&m_buf[m_offset], data, bytes);
    m_offset += bytes;
    // if (m_offset > m_bytes)
    //  m_bytes = m_offset;

    return bytes == fwrite(data, 1, bytes, m_file) ? 0 : ferror(m_file);
}

int CMovBufFile::Seek(int64_t offset) {
    if ((offset >= 0 ? offset : -offset) >= m_maxsize) {
        LOG_ERROR("seek:%lld, m_maxsize:%zu", offset, m_maxsize);
        return -E2BIG;
    }

    m_offset = (size_t)(offset >= 0 ? offset : m_maxsize + offset);
    ZC_ASSERT(offset <= (int64_t)m_buf.capacity());
    // LOG_WARN("seek, offset%zu", m_offset);
    return fseek64(m_file, offset, offset >= 0 ? SEEK_SET : SEEK_END);
}

int64_t CMovBufFile::Tell() {
    // LOG_WARN("tell, offset%zu, %p", m_offset, m_file);
    // int64_t fsize = ftell64(m_file);
    // if (fsize != m_offset) {
    //     LOG_ERROR("error, fsize, m_offset, fsize:%lld, %zu:", fsize, m_offset);
    //     ZC_ASSERT(0);  // for debug
    // }
    return (int64_t)m_offset;
    // return ftell64(m_file);
}

CMovIo *CMovIoFac::CMovIoCreate(const zc_movio_info_t &info) {
    return CMovIoCreate(info.type, &info.uninfo);
}

CMovIo *CMovIoFac::CMovIoCreate(fmp4_movio_e type, const zc_movio_info_un *uninfo /*= nullptr*/) {
    CMovIo *io = nullptr;
    LOG_TRACE("CMovIoCreate, type:%d", type);
    switch (type) {
    case fmp4_movio_buf:
        io = new CMovBuf((zc_movio_bufinfo_t *)uninfo);
        break;
    case fmp4_movio_file: {
        io = new CMovFile((zc_movio_fileinfo_t *)uninfo);
        break;
    }
    case fmp4_movio_buffile: {
        io = new CMovBufFile((zc_movio_buffileinfo_t *)uninfo);
        break;
    }
    default:
        LOG_ERROR("error, code[%d]", type);
        break;
    }
    return io;
}

CMovReadIo::CMovReadIo(const char *name) {
    m_file = fopen(name, "rb");
    if (!m_file) {
        LOG_ERROR("error open:%s", name);
        return;
    }
    m_io.read = ioRead;
    m_io.write = nullptr;
    m_io.seek = ioSeek;
    m_io.tell = ioTell;
    m_name = name;
    LOG_TRACE("open open:%s", name);
}

CMovReadIo::~CMovReadIo() {
    Close();
}

void CMovReadIo::Close() {
    if (m_file) {
        fsync(fileno(m_file));
        fclose(m_file);
        m_file = nullptr;
        LOG_TRACE("close:%s", m_name.c_str());
    }
    return;
}

int CMovReadIo::Read(void *data, uint64_t bytes) {
    if (bytes == fread(data, 1, bytes, m_file))
        return 0;
    return 0 != ferror(m_file) ? ferror(m_file) : -1 /*EOF*/;
}

int CMovReadIo::Write(const void *data, uint64_t bytes) {
    return bytes == fwrite(data, 1, bytes, m_file) ? 0 : ferror(m_file);
}

int CMovReadIo::Seek(int64_t offset) {
    return fseek64(m_file, offset, offset >= 0 ? SEEK_SET : SEEK_END);
}

int64_t CMovReadIo::Tell() {
    return ftell64(m_file);
}

}  // namespace zc