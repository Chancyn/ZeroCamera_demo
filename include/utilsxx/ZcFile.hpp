// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <string>

namespace zc {

class CFile {
 public:
    CFile();
    virtual ~CFile();
 public:
    bool Open(const char *name, const char *mode);
    bool Close();
    virtual size_t Read(void *data, size_t bytes);
    virtual size_t Write(const void *data, size_t bytes);
    virtual int64_t Seek(int64_t offset);
    virtual int64_t Tell();

 private:
    std::string m_name;
    FILE *m_file;
};

}
