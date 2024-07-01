// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <string.h>

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcMsg.hpp"
#include "ZcMsgProcMod.hpp"
#include "ZcType.hpp"
#include "sys/ZcMsgProcModSys.hpp"

namespace zc {
CMsgProcModSys::CMsgProcModSys() : CMsgProcMod(ZC_MODID_SYS_E, ZC_MID_SYS_BUTT), m_init(false) {
    LOG_TRACE("Constructor into");
}

CMsgProcModSys::~CMsgProcModSys() {
    LOG_TRACE("Destructor into");
    UnInit();
}

ZC_S32 CMsgProcModSys::_handleRepNull(zc_msg_t *rep, int size) {
    // do noting
    return 0;
}

// Manager
ZC_S32 CMsgProcModSys::_handleReqSysManVersion(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManVersion,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManVersion(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManVersion,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManRegister,iqsize[%d], pid:%d,modid:%u", iqsize, req->pid, req->modid);
    ZC_ASSERT(iqsize == sizeof(zc_msg_t) + sizeof(zc_mod_reg_t));
    ZC_ASSERT(*opsize >= sizeof(zc_msg_t));

    zc_mod_reg_t *msgreq = reinterpret_cast<zc_mod_reg_t *>(req->data);

    *opsize = sizeof(zc_msg_t);
    int ret = 0;
    if (m_cbinfo.MgrHandleCb) {
        m_cbinfo.MgrHandleCb(m_cbinfo.MgrContext, 0, nullptr, nullptr);
    }
    LOG_TRACE("222 handle ReqSysManRegister,iqsize[%d], pid:%d,modid:%u", iqsize, req->pid, req->modid);
    LOG_TRACE("handle ReqSysManRegister,ret:%d, cmd:%d, url:%s,date:%s ", rep->err, msgreq->regcmd, msgreq->url,
              msgreq->date);
    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManRegister(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManRegister,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManRestart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManRestart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManRestart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManRestart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManShutdown(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysManShutdown,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManShutdown(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysManShutdown,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysManKeepalive(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle _handleReqSysManKeepalive,iqsize[%d]", iqsize);
    ZC_ASSERT(iqsize = sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_t));
    ZC_ASSERT(*opsize >= sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_rep_t));

    zc_mod_keepalive_t *msgreq = reinterpret_cast<zc_mod_keepalive_t *>(req->data);
    zc_mod_keepalive_rep_t *msgrep = reinterpret_cast<zc_mod_keepalive_rep_t *>(rep->data);

    *opsize = sizeof(zc_msg_t) + sizeof(zc_mod_keepalive_rep_t);
    msgrep->seqno = msgrep->seqno;
    msgrep->status = 0;
    strncpy(msgrep->date, msgreq->date, sizeof(msgrep->date) - 1);
    LOG_TRACE("handle keepalive pid[%d] seqno[%u] status[%d] date:%s->%s", req->pid, req->modid, msgreq->seqno,
              msgreq->status, msgreq->date, msgreq->date);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysManKeepalive(zc_msg_t *rep, int size) {
    LOG_TRACE("handle _handleRepSysManKeepalive,size[%d]", size);
    zc_mod_keepalive_t *pkeepalive = reinterpret_cast<zc_mod_keepalive_t *>(rep->data);
    LOG_TRACE("handle keepalive modid[%u] seqno[%u] status[%d]", rep->modid, pkeepalive->seqno, pkeepalive->status);

    return 0;
}

// streamMgr
ZC_S32 CMsgProcModSys::_handleReqSysSMgrRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysSMgrRegister,iqsize[%d]", iqsize);
    zc_mod_smgr_reg_t *msg = reinterpret_cast<zc_mod_smgr_reg_t *>(req->data);
    LOG_TRACE("handle Register pid:%d, modid:%u, p status[%d]", req->pid, req->modid, msg->status);
    int ret = 0;
    if (m_cbinfo.streamMgrHandleCb) {
        zc_sys_smgr_reg_t hmsg = {0};
        hmsg.modid = req->modid;
        hmsg.pid = req->pid;
        hmsg.status = msg->status;
        strncpy(hmsg.pname, msg->pname, sizeof(hmsg.pname));
        ret = m_cbinfo.streamMgrHandleCb(m_cbinfo.streamMgrContext, 0, &hmsg, nullptr);
        *opsize = sizeof(zc_msg_t);
    }

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysSMgrRegister(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSMgrRegister,size[%d]", size);
    zc_mod_smgr_reg_t *msg = reinterpret_cast<zc_mod_smgr_reg_t *>(rep->data);
    LOG_TRACE("handle Register pid:%d, modid:%u, status[%d]", rep->pid, rep->modid, msg->status);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysSMgrUnRegister(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSMgrUnRegister,iqsize[%d]", iqsize);
    zc_mod_smgr_unreg_t *msg = reinterpret_cast<zc_mod_smgr_unreg_t *>(req->data);
    LOG_TRACE("handle UnRegister modid:%u,pid:%u, status:%d", msg->modid, msg->pid, msg->status);

    if (m_cbinfo.streamMgrHandleCb) {
        m_cbinfo.streamMgrHandleCb(m_cbinfo.streamMgrContext, 0, nullptr, nullptr);
        *opsize = sizeof(zc_msg_t);
    }

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysSMgrUnRegister(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSMgrUnRegister,size[%d]", size);
    zc_mod_smgr_unreg_t *msg = reinterpret_cast<zc_mod_smgr_unreg_t *>(rep->data);
    LOG_TRACE("handle UnRegister modid:%u, pid:%u[%s], status:%d", msg->modid, msg->pid, msg->pname, msg->status);

    return 0;
}

static inline void _smgr_iteminfo_trans(zc_mod_smgr_iteminfo_t *modinfo, const zc_shmstream_info_t *info) {
    modinfo->shmstreamtype = info->shmstreamtype;
    modinfo->chn = info->chn;
    modinfo->idx = info->idx;
    modinfo->tracknum = info->tracknum;
    modinfo->status = info->status;

    for (unsigned int i = 0; i < info->tracknum && i < ZC_MSG_TRACK_MAX_NUM; i++) {
        modinfo->tracks[i].chn = info->tracks[i].chn;
        modinfo->tracks[i].trackno = info->tracks[i].trackno;
        modinfo->tracks[i].tracktype = info->tracks[i].tracktype;
        modinfo->tracks[i].encode = info->tracks[i].encode;
        modinfo->tracks[i].enable = info->tracks[i].enable;
        modinfo->tracks[i].fifosize = info->tracks[i].fifosize;
        modinfo->tracks[i].framemaxlen = info->tracks[i].framemaxlen;
        modinfo->tracks[i].status = info->tracks[i].status;
        strncpy(modinfo->tracks[i].name, info->tracks[i].name, sizeof(modinfo->tracks[i].name) - 1);
    }
    // _dumpStreamInfo("user", modinfo);
    return;
}

ZC_S32 CMsgProcModSys::_handleReqSysSMgrGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysSMgrGet,iqsize[%d]", iqsize);
    ZC_ASSERT(iqsize = sizeof(zc_msg_t) + sizeof(zc_mod_smgr_get_t));
    ZC_ASSERT(*opsize >= sizeof(zc_msg_t) + sizeof(zc_mod_smgr_get_rep_t));
    zc_mod_smgr_get_t *reqmsg = reinterpret_cast<zc_mod_smgr_get_t *>(req->data);
    if (m_cbinfo.streamMgrHandleCb) {
        zc_sys_smgr_getinfo_in_t stin = {0};
        zc_sys_smgr_getinfo_out_t stout = {0};
        stin.type = reqmsg->type;
        stin.chn = reqmsg->chn;
        if (m_cbinfo.streamMgrHandleCb(m_cbinfo.streamMgrContext, SYS_SMGR_HDL_GETINFO_E, &stin, &stout) < 0) {
            return ZC_MSG_ERR_HADNLE_E;
        }
        zc_mod_smgr_get_rep_t *repmsg = reinterpret_cast<zc_mod_smgr_get_rep_t *>(rep->data);
        _smgr_iteminfo_trans(&repmsg->info, &stout.info);
        *opsize = sizeof(zc_msg_t)+ sizeof(zc_mod_smgr_get_rep_t);
        return ZC_MSG_SUCCESS_E;
    }

    return ZC_MSG_ERR_E;
}

ZC_S32 CMsgProcModSys::_handleReqSysSMgrGetAll(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysSMgrGet,iqsize[%d]", iqsize);
    ZC_ASSERT(iqsize = sizeof(zc_msg_t) + sizeof(zc_mod_smgr_getall_t));
    ZC_ASSERT(*opsize >= sizeof(zc_msg_t) + sizeof(zc_mod_smgr_getall_rep_t));
    int ret = ZC_MSG_ERR_E;
    int size = sizeof(zc_msg_t)+ sizeof(zc_mod_smgr_getall_rep_t);
    zc_mod_smgr_getall_t *reqmsg = reinterpret_cast<zc_mod_smgr_getall_t *>(req->data);
    if (m_cbinfo.streamMgrHandleCb) {
        zc_sys_smgr_getcount_in_t stcountin = {0};
        zc_sys_smgr_getcount_out_t stcountout = {0};
        stcountin.type = reqmsg->type;
        if (m_cbinfo.streamMgrHandleCb(m_cbinfo.streamMgrContext, SYS_SMGR_HDL_GECOUNT_E, &stcountin, &stcountout) <
            0) {
            LOG_ERROR("SYS_SMGR_HDL_GETCOUNT_E error");
            return ZC_MSG_ERR_HADNLE_E;
        }

        // calcu max count
        int count = ((*opsize) - sizeof(zc_msg_t) - sizeof(zc_mod_smgr_getall_rep_t)) / sizeof(zc_mod_smgr_iteminfo_t);
        if (count < stcountout.count) {
            stcountout.count = count;
            LOG_WARN("count %d < %u", count, stcountout.count);
        }
        LOG_WARN("count %u", stcountout.count);
        zc_mod_smgr_getall_rep_t *repmsg = reinterpret_cast<zc_mod_smgr_getall_rep_t *>(rep->data);
        repmsg->itemcount = stcountout.count;
        repmsg->itemsize = sizeof(zc_mod_smgr_iteminfo_t);
        size += repmsg->itemsize *  repmsg->itemcount;
        *opsize = size;
        if (stcountout.count > 0) {
            zc_sys_smgr_getallinfo_in_t stin = {0};
            zc_sys_smgr_getallinfo_out_t stout = {0};
            stout.pinfo = new zc_shmstream_info_t[stcountout.count]();
            ZC_ASSERT(stout.pinfo != nullptr);
            stin.type = reqmsg->type;
            stin.count = stcountout.count;
            if (m_cbinfo.streamMgrHandleCb(m_cbinfo.streamMgrContext, SYS_SMGR_HDL_GETALLINFO_E, &stin, &stout) < 0) {
                ret = ZC_MSG_ERR_HADNLE_E;
            } else {
                zc_mod_smgr_iteminfo_t *item =
                    reinterpret_cast<zc_mod_smgr_iteminfo_t *>(rep->data + sizeof(zc_mod_smgr_getall_rep_t));
                for (unsigned int i = 0; i < stcountout.count; i++) {
                    _smgr_iteminfo_trans(item + i, stout.pinfo + i);
                }
                ret = ZC_MSG_SUCCESS_E;
            }
            delete[](stout.pinfo);
        }
    }

    return ret;
}

ZC_S32 CMsgProcModSys::_handleRepSysSMgrGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSMgrGet,size[%d]", size);
    zc_mod_smgr_get_t *pGet = reinterpret_cast<zc_mod_smgr_get_t *>(rep->data);
    LOG_TRACE("handle Get");

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysSMgrSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSMgrSet,iqsize[%d]", iqsize);
    zc_mod_smgr_set_t *pSet = reinterpret_cast<zc_mod_smgr_set_t *>(req->data);
    LOG_TRACE("handle Set");

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysSMgrSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepStreamMgSet,size[%d]", size);
    zc_mod_smgr_set_t *pSet = reinterpret_cast<zc_mod_smgr_set_t *>(rep->data);
    LOG_TRACE("handle Set");

    return 0;
}

// Time
ZC_S32 CMsgProcModSys::_handleReqSysTimeGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysTimeGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysTimeGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysTimeGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysTimeSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysTimeSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysTimeSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysTimeSet,size[%d]", size);

    return 0;
}

// Base
ZC_S32 CMsgProcModSys::_handleReqSysBaseGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysBaseGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysBaseGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysBaseGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysBaseSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysBaseSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysBaseSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysBaseSet,size[%d]", size);

    return 0;
}

// User
ZC_S32 CMsgProcModSys::_handleReqSysUserGet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUserGet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUserGet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUserGet,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysUserSet(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUserSet,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUserSet(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUserSet,size[%d]", size);

    return 0;
}

// upgrade
ZC_S32 CMsgProcModSys::_handleReqSysUpgStart(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUpgStart,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUpgStart(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUpgStart,size[%d]", size);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleReqSysUpgStop(zc_msg_t *req, int iqsize, zc_msg_t *rep, int *opsize) {
    LOG_TRACE("handle ReqSysUpgStop,iqsize[%d]", iqsize);

    return 0;
}

ZC_S32 CMsgProcModSys::_handleRepSysUpgStop(zc_msg_t *rep, int size) {
    LOG_TRACE("handle RepSysUpgStop,size[%d]", size);

    return 0;
}

bool CMsgProcModSys::Init(void *cbinfo) {
    if (m_init) {
        LOG_ERROR("already init");
        return false;
    }

    // TODO(zhoucc) register all msgfunction
    // ZC_MID_SYS_MAN_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_REGISTER_E, &CMsgProcModSys::_handleReqSysManRegister,
                 &CMsgProcModSys::_handleRepSysManRegister);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_VERSION_E, &CMsgProcModSys::_handleReqSysManVersion,
                 &CMsgProcModSys::_handleRepSysManVersion);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_RESTART_E, &CMsgProcModSys::_handleReqSysManRestart,
                 &CMsgProcModSys::_handleRepSysManRestart);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_SHUTDOWN_E, &CMsgProcModSys::_handleReqSysManShutdown,
                 &CMsgProcModSys::_handleRepSysManShutdown);
    REGISTER_MSG(m_modid, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_KEEPALIVE_E, &CMsgProcModSys::_handleReqSysManKeepalive,
                 &CMsgProcModSys::_handleRepSysManKeepalive);

    // ZC_MID_SYS_SMGR_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_REGISTER_E, &CMsgProcModSys::_handleReqSysSMgrRegister,
                 &CMsgProcModSys::_handleRepSysSMgrRegister);
    REGISTER_MSG(m_modid, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_UNREGISTER_E, &CMsgProcModSys::_handleReqSysSMgrUnRegister,
                 &CMsgProcModSys::_handleRepSysSMgrUnRegister);
    REGISTER_MSG(m_modid, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_GET_E, &CMsgProcModSys::_handleReqSysSMgrGet,
                 &CMsgProcModSys::_handleRepSysSMgrGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_SET_E, &CMsgProcModSys::_handleReqSysSMgrSet,
                 &CMsgProcModSys::_handleRepSysSMgrSet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_SMGR_E, ZC_MSID_SMGR_GETALL_E, &CMsgProcModSys::_handleReqSysSMgrGetAll,
                 &CMsgProcModSys::_handleRepNull);

    // ZC_MID_SYS_TIME_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_TIME_E, ZC_MSID_SYS_TIME_GET_E, &CMsgProcModSys::_handleReqSysTimeGet,
                 &CMsgProcModSys::_handleRepSysTimeGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_TIME_E, ZC_MSID_SYS_TIME_SET_E, &CMsgProcModSys::_handleReqSysTimeSet,
                 &CMsgProcModSys::_handleRepSysTimeSet);

    // ZC_MID_SYS_BASE_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_BASE_E, ZC_MSID_SYS_BASE_GET_E, &CMsgProcModSys::_handleReqSysBaseGet,
                 &CMsgProcModSys::_handleRepSysBaseGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_BASE_E, ZC_MSID_SYS_BASE_SET_E, &CMsgProcModSys::_handleReqSysBaseSet,
                 &CMsgProcModSys::_handleRepSysBaseSet);

    // ZC_MID_SYS_USER_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_USER_E, ZC_MSID_SYS_USER_GET_E, &CMsgProcModSys::_handleReqSysUserGet,
                 &CMsgProcModSys::_handleRepSysUserGet);
    REGISTER_MSG(m_modid, ZC_MID_SYS_USER_E, ZC_MSID_SYS_USER_SET_E, &CMsgProcModSys::_handleReqSysUserSet,
                 &CMsgProcModSys::_handleRepSysUserSet);

    // ZC_MID_SYS_UPG_E
    REGISTER_MSG(m_modid, ZC_MID_SYS_UPG_E, ZC_MSID_SYS_UPG_START_E, &CMsgProcModSys::_handleReqSysUpgStart,
                 &CMsgProcModSys::_handleRepSysUpgStart);
    REGISTER_MSG(m_modid, ZC_MID_SYS_UPG_E, ZC_MSID_SYS_UPG_STOP_E, &CMsgProcModSys::_handleReqSysUpgStop,
                 &CMsgProcModSys::_handleRepSysUpgStop);
    init();

    if (cbinfo) {
        memcpy(&m_cbinfo, cbinfo, sizeof(m_cbinfo));
    }

    m_init = true;

    LOG_TRACE("Init ok");
    return true;
}

bool CMsgProcModSys::UnInit() {
    if (!m_init) {
        LOG_ERROR("not init");
        return false;
    }

    uninit();
    m_init = false;
    LOG_TRACE("UnInit ok");
    return true;
}

}  // namespace zc
