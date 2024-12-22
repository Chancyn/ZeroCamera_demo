// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include "zc_type.h"

#define ZC_SAFE_DELETE(x) \
    do { \
        if ((x) != nullptr) { \
            delete (x); \
            x = nullptr; \
        } \
    } while (0)

#define ZC_SAFE_DELETEA(x) \
    do { \
        if ((x) != nullptr) { \
            delete[] (x); \
            x = nullptr; \
        } \
    } while (0)
