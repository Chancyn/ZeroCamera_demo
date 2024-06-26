// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#include "sys/ZcModSys.hpp"

namespace zc {
class CSysManager : public CModSys {
 public:
    CSysManager();
    virtual ~CSysManager();

 public:
    bool Init(sys_callback_info_t *cbinfo);
    bool UnInit();
    bool Start();
    bool Stop();

 private:
    bool _unInit();

 private:
    bool m_init;
    int m_running;
};
}  // namespace zc
