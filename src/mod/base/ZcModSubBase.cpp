// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <functional>

#include "zc_log.h"
#include "zc_macros.h"
#include "zc_msg.h"
#include "zc_msg_sys.h"
#include "zc_type.h"

#include "ZcType.hpp"
#include "ZcModSubBase.hpp"
#include "ZcModBase.hpp"

namespace zc {
CModSubBase::CModSubBase(ZC_U8 modid): CModBase(modid) {
}

CModSubBase::~CModSubBase() {

}
}  // namespace zc
