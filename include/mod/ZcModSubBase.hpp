// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "ZcMsg.hpp"

namespace zc {
class CModSubBase : public CModBase {
 public:
    CModSubBase(ZC_U8 modid);
    virtual ~CModSubBase();

 private:
    int m_init;
};
}  // namespace zc
