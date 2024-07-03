// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "Singleton.hpp"
#include "zc_type.h"

#include "sys/ZcModSys.hpp"

namespace zc {
class CSysManager : public CModSys, public Singleton<CSysManager> {
 public:
    CSysManager();
    virtual ~CSysManager();

 public:
    bool Init(sys_callback_info_t *cbinfo);
    bool UnInit();
    bool Start();
    bool Stop();
    static int PublishMsgCb(void *ptr, ZC_U16 smid, ZC_U16 smsid, void *msg, unsigned int len);
    int PublishMsg(ZC_U16 smid, ZC_U16 smsid, void *msg, unsigned int len);
 private:
    bool _unInit();

 private:
    bool m_init;
    int m_running;
};
#define g_ZCSysManagerInstance (zc::CSysManager::GetInstance())
}  // namespace zc
