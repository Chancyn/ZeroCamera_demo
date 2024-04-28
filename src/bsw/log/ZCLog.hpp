// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <memory>

#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include "Singleton.hpp"

// config
typedef struct _zc_logconf_ {
    int level;
    int fulshlevel;
    int conslevel;
    int64_t size;
    int count;
    char path[256];
} zc_logconf_t;

#define _baselog(logger, level, ...) (logger)->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, level, __VA_ARGS__)
#define logxx_levelout(level, ...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), level, __VA_ARGS__)

// log for cxx;
#define logxx_trace(...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), spdlog::level::trace, __VA_ARGS__)
#define logxx_debug(...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), spdlog::level::debug, __VA_ARGS__)
#define logxx_info(...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), spdlog::level::info, __VA_ARGS__)
#define logxx_warn(...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), spdlog::level::warn, __VA_ARGS__)
#define logxx_error(...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), spdlog::level::err, __VA_ARGS__)
#define logxx_criti(...) _baselog(ZCLog::GetInstance().GetLoggerPtr(), spdlog::level::critical, __VA_ARGS__)

// Singleton
class ZCLog : public Singleton<ZCLog> {
 public:
    ZCLog() : m_binit(false) { memset(&m_LogCfg, 0, sizeof(m_LogCfg)); }
    virtual ~ZCLog() {}
    std::shared_ptr<spdlog::logger> GetLoggerPtr() { return loggerPtr; }

    void Init(const zc_logconf_t &conf);
    void UnInit();
    int GetLogLevel();
    void SetLogLevel(int level);

 private:
    std::shared_ptr<spdlog::logger> loggerPtr;
    zc_logconf_t m_LogCfg;
    bool m_binit = false;
};

#define g_ZCLogInstance (ZCLog::GetInstance())
