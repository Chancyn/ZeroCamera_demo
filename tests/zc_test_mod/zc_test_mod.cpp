// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "zc_log.h"
#include "zc_msg.h"
#include "zc_msg_rtsp.h"
#include "zc_msg_sys.h"

#include "Timer.hpp"
#include "ZcModFac.hpp"
#include "ZcType.hpp"
#include "zc_test_mod.hpp"

zc::CModBase *g_ModTab[ZC_MODID_BUTT] = {};

static int zc_test_mod_sendreg(int modid, int modidto) {
    char msg_buf[sizeof(zc_msg_t) + sizeof(zc_mod_reg_t)] = {0};
    zc_msg_t *pmsg = reinterpret_cast<zc_msg_t *>(msg_buf);

    g_ModTab[modid]->BuildReqMsgHdr(pmsg, modidto, ZC_MID_SYS_MAN_E, ZC_MSID_SYS_MAN_REGISTER_E, 0,
                                    sizeof(zc_mod_reg_t));
    g_ModTab[modid]->MsgSendTo(pmsg, ZC_SYS_URL_IPC);
    // g_ModTab[modid]->MsgSendTo(pmsg);

    return 0;
}

// start
int zc_test_mod_start(int modid) {
    modid %= ZC_MODID_BUTT;
    LOG_INFO("test_mod modid[%d] start into\n", modid);
    zc::CModFac fac;
    g_ModTab[modid] = fac.CreateMod(modid);
    if (!g_ModTab[modid]) {
        LOG_ERROR("test_mod modid[%d] Create error\n", modid);
        return false;
    }
    g_ModTab[modid]->Init();

    if (modid != 0) {
        LOG_ERROR("zc_test_mod_sendreg modid[%d] \n", modid);
        // zc_test_mod_sendreg(modid, 0);
    }
    LOG_INFO("test_mod modid[%d] start[%p] end\n", modid, g_ModTab[modid]);
    return 0;
}

// stop
int zc_test_mod_stop(int modid) {
    modid %= ZC_MODID_BUTT;
    LOG_INFO("test_mod modid[%d] stop[%p] into\n", modid, g_ModTab[modid]);
    if (g_ModTab[modid]) {
        g_ModTab[modid]->UnInit();
    }

    ZC_SAFE_DELETE(g_ModTab[modid]);
    LOG_INFO("test_mod modid[%d] stop end\n", modid);

    return 0;
}
