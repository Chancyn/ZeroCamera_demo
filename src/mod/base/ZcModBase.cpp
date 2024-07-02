// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <functional>

#include "rtsp/zc_rtsp_smgr_handle.h"
#include "zc_basic_fun.h"
#include "zc_log.h"
#include "zc_macros.h"
#include "zc_mod_base.h"
#include "zc_msg_codec.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"
#include "zc_proc.h"
#include "zc_type.h"

#include "ZcModBase.hpp"
#include "ZcModCli.hpp"
#include "ZcType.hpp"

namespace zc {
#if 0
CModBase::CModBase(ZC_U8 modid, ZC_U32 version)
    : CModCli(modid), Thread(std::string(CModCli::GetUrlbymodid(modid))), m_init(false),
      m_modid(modid), m_seqno(0), m_version(version) {
    m_pid = getpid();
    strncpy(m_url, CModCli::GetUrlbymodid(modid), sizeof(m_url) - 1);
    strncpy(m_name, CModCli::GetUrlbymodid(modid), sizeof(m_name) - 1);
    ZC_PROC_GETNAME(m_pname, sizeof(m_pname));
}
#else
CModBase::CModBase(ZC_U8 modid, ZC_U32 version)
    : CModCli(modid), Thread(std::string(CModCli::GetUrlbymodid(modid))), m_init(false){
    strncpy(m_url, CModCli::GetUrlbymodid(modid), sizeof(m_url) - 1);
     ZC_PROC_GETNAME(m_pname, sizeof(m_pname));
}
#endif
CModBase::~CModBase() {
    unInitReqSvr();
}

void CModBase::BuildRepMsgHdr(zc_msg_t *rep, zc_msg_t *req) {
    memcpy(rep, req, sizeof(zc_msg_t));
    rep->msgtype = ZC_MSG_TYPE_REP_E;
    rep->ts1 = zc_system_time();
    return;
}

int CModBase::_checklicense() {
    if (unlikely(m_syslicstatus == SYS_LIC_STATUS_TEMP_LIC_E)) {
        time_t now = time(NULL);
        if (now > m_expire) {
            m_syslicstatus = SYS_LIC_STATUS_EXPIRED_LIC_E;
            LOG_ERROR("license timeout now:%u > %u", now, m_expire);
        }
    }

    return m_syslicstatus;
}

void CModBase::_initlicense() {
    // TODO(zhoucc): load license
    m_inittime = time(NULL);
    m_syslicstatus = SYS_LIC_STATUS_SUC_E;
    m_expire = m_inittime + ZC_MOD_LIC_EXPIRE_TIME;
    LOG_TRACE("modid:%d init license:%d", m_modid, m_syslicstatus);
    ZC_ASSERT(m_syslicstatus != SYS_LIC_STATUS_ERR_E);

    return;
}

bool CModBase::initReqSvr(MsgCommReqSerHandleCb svrcb) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // init license
    _initlicense();
    if (!InitComm(m_url, svrcb)) {
        LOG_ERROR("InitComm error modid:%d, url:%s", m_modid, m_url);
        goto _err;
    }

    m_init = true;
    LOG_TRACE("init ok modid:%d, url:%s", m_modid, m_url);
    return true;

_err:
    LOG_ERROR("Init error");
    return false;
}

bool CModBase::unInitReqSvr() {
    if (!m_init) {
        return true;
    }

    UnInitComm();
    m_init = false;
    LOG_TRACE("unInit ok");
    return false;
}

zc_msg_errcode_e CModBase::MsgReqProc(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    // TODO(zhoucc) find msg procss mod
    if (m_pmsgmodproc) {
        return (zc_msg_errcode_e)m_pmsgmodproc->MsgReqProc(req, iqsize, rep, opsize);
    }

    return ZC_MSG_ERR_E;
}

bool CModBase::registerMsgProcMod(CMsgProcMod *msgprocmod) {
    // uinit status can register msgprocmod
    if (m_init) {
        return false;
    }

    // TODO(zhoucc) support register one or more msgprocmod
    m_pmsgmodproc = msgprocmod;

    return true;
}

bool CModBase::unregisterMsgProcMod(CMsgProcMod *msgprocmod) {
    // uinit status can register msgprocmod
    if (m_init) {
        return false;
    }

    // TODO(zhoucc) : support more msgprocmod
    m_pmsgmodproc = nullptr;

    return false;
}

int CModBase::process() {
    return modprocess();
}
}  // namespace zc
