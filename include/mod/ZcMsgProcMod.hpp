// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "ZcMsg.hpp"
#include "zc_mod_base.h"

namespace zc {
#define ZC_MOD_LIC_EXPIRE_TIME (24 * 3600)  // temporary license expired time

// license status
typedef enum {
    SYS_LIC_STATUS_EXPIRED_LIC_E = -2,  // expired license
    SYS_LIC_STATUS_ERR_E = -1,          // license error assert
    SYS_LIC_STATUS_INIT_E = 0,          // unload license
    SYS_LIC_STATUS_TEMP_LIC_E,          // temporary license, for debug test
    SYS_LIC_STATUS_SUC_E,               // success perpetual register

    SYS_LIC_STATUS_BUTT_E,
} sys_lic_status_e;

class CMsgProcMod {
 public:
    CMsgProcMod(ZC_U8 modid, ZC_U16 idmax);
    virtual ~CMsgProcMod();
    virtual bool Init(void *cbinfo) = 0;
    virtual bool UnInit() = 0;
    ZC_S32 MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize);
    ZC_S32 MsgRepProc(zc_msg_t *rep, int size);

 protected:
    bool registerMsg(ZC_U16 id, ZC_U16 sid, CMsgBase *pMsg);
    bool init();
    bool uninit();

 private:
    // first init license
    void _initlicense();
    int _checklicense();

 protected:
    ZC_U8 m_modid;
    ZC_U16 m_idmax;
    std::map<ZC_U32, CMsgBase *> m_msgmap;

 private:
    int m_init;
    ZC_U32 m_expire;                  // license expire time
    ZC_U32 m_inittime;                // license load time
    sys_lic_status_e m_syslicstatus;  // license status
};
}  // namespace zc
