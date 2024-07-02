// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "zc_mod_base.h"

#include "ZcModBase.hpp"
#include "ZcMsg.hpp"

namespace zc {
class CModSysBase : public CModBase {
 public:
    CModSysBase();
    virtual ~CModSysBase();

 private:
    int m_init;
};
}  // namespace zc
