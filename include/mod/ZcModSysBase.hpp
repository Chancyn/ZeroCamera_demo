// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <memory>

#include "ZcModPubSub.hpp"
#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "ZcMsg.hpp"

namespace zc {
class CModSysBase : public CModBase, public CModPublish {
 public:
    CModSysBase();
    virtual ~CModSysBase();
    bool init();
    bool unInit();
    bool Start();
    bool Stop();

 private:
    bool initPubSvr();
    bool unInitPubSvr();
    zc_msg_errcode_e checkReqSvrRecvReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 reqSvrRecvReqProc(char *req, int iqsize, char *rep, int *opsize);
    bool registerInsert(zc_msg_t *msg);
    bool unregisterRemove(zc_msg_t *msg);
    bool updateStatus(zc_msg_t *msg);
    int updateStatus(ZC_S32 pid);
    int _sysCheckModCliStatus();

    virtual int modprocess();

 private:
    bool m_init;

    // ZC_U64 = (pid << 32) | modid;
    std::map<ZC_U64, std::shared_ptr<sys_modcli_status_t>> m_modmap;  // modclimap
    std::mutex m_mutex;                                               // map lock
};
}  // namespace zc
