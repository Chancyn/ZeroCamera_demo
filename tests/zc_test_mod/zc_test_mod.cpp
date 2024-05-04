// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "Timer.hpp"
#include "ZcType.hpp"
#include "zc_log.h"

#include "ZcModFac.hpp"
#include "zc_test_mod.hpp"

zc::CModBase *g_ModTab[ZC_MODID_BUTT] = {};

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
