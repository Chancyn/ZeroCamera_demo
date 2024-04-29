// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

// raw ptr
class CObserver {
 public:
    CObserver() {}
    virtual ~CObserver() {}

    // notify
    virtual void Notify(ZC_U32 u32MsgId, ZC_U32 u32SubMsgId, void *pData, ZC_U32 u32DataLen) = 0;
};
