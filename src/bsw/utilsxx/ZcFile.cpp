// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "zc_log.h"

#include "ZcFile.hpp"

namespace zc {
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

CFile::CFile() : m_file(nullptr) {}

CFile::~CFile() {
    Close();
}

bool CFile::Open(const char *name, const char *mode) {
    m_name = name;
    m_file = fopen(m_name.c_str(), "wb+");
    if (m_file == nullptr) {
        LOG_ERROR("error open:%s", m_name.c_str());
        m_name.clear();
        return false;
    }

    LOG_TRACE("open:%s ok", m_name.c_str());
    return true;
}

bool CFile::Close() {
    if (m_file) {
        fsync(fileno(m_file));
        fclose(m_file);
        m_file = nullptr;
        LOG_TRACE("close:%s", m_name.c_str());
    }
    return true;
}

size_t CFile::Read(void *data, size_t bytes) {
    if (bytes == fread(data, 1, bytes, m_file))
        return 0;
    return 0 != ferror(m_file) ? ferror(m_file) : -1 /*EOF*/;
}

size_t CFile::Write(const void *data, size_t bytes) {
    return bytes == fwrite(data, 1, bytes, m_file) ? 0 : ferror(m_file);
}

int64_t CFile::Seek(int64_t offset) {
    return fseek64(m_file, offset, offset >= 0 ? SEEK_SET : SEEK_END);
}

int64_t CFile::Tell() {
    return ftell64(m_file);
}

}  // namespace zc