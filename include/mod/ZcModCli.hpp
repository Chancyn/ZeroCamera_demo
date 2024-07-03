// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <memory>

#include "zc_mod_base.h"
#include "zc_msg.h"
#include "zc_type.h"
#include "zc_frame.h"

#include "ZcModReqCli.hpp"
#include "ZcModPubSub.hpp"

namespace zc {

class CModCli : public CModReqCli, public CModSubscriber {
 public:
    explicit CModCli(ZC_U8 modid, ZC_U32 version = ZC_MSG_VERSION);
    virtual ~CModCli();
    bool initSubCli(MsgCommSubCliHandleCb handl);
    bool unInitSubCli();
    bool Init(MsgCommSubCliHandleCb handle);
    bool UnInit();
};
}  // namespace zc
