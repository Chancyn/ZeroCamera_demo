// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <iostream>
#include <vector>

#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#include "ZCLog.hpp"

// format[%Y-%m-%d %H:%M:%S.%e] 时间 [%l] 日志级别 [%t] 线程 [%s] 文件 [%#] 行号 [%!] 函数 [%v] 实际文本
// format #define ZC_LOG_FMT "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] [%s %!:%#] %v"
#define ZC_LOG_FMT "[%Y-%m-%d %H:%M:%S.%e][%^%l%$][tid %t] %v"

void ZCLog::Init(const zc_logconf_t &conf) {
    if (m_binit) {
        logxx_levelout(spdlog::level::level_enum::warn, "already init");
        return;
    }
    try {
        /* 通过multi-sink的方式创建复合logger*/
        /* file sink */
        spdlog::sink_ptr file_sink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(conf.path, conf.size, conf.count);
        // file_sink->set_level(spdlog::level::trace);
        printf("zhoucc  level [%d]", conf.level);
        file_sink->set_level((spdlog::level::level_enum)(conf.level));
        file_sink->set_pattern(ZC_LOG_FMT);
#if ZC_DEBUG
        printf("zhoucc conslevel [%d]", conf.conslevel);
        /* 控制台sink */
        // spdlog::sink_ptr console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        console_sink->set_level((spdlog::level::level_enum)(conf.conslevel));
        console_sink->set_pattern(ZC_LOG_FMT);
#endif

        /* Sink组合 */
        std::vector<spdlog::sink_ptr> sinks;

#if ZC_DEBUG
        sinks.push_back(console_sink);
#endif
        sinks.push_back(file_sink);
        loggerPtr = std::make_shared<spdlog::logger>("multi-sink", begin(sinks), end(sinks));  // NOLINT

        // flush level disk
        printf("zhoucc fulsh level [%d]", conf.fulshlevel);
        loggerPtr->flush_on((spdlog::level::level_enum)(conf.fulshlevel));
        loggerPtr->set_level((spdlog::level::level_enum)(conf.conslevel));
        std::cout << "SPDLOG: create spdlog success!" << std::endl;
        memcpy(&m_LogCfg, &conf, sizeof(m_LogCfg));
        m_binit = true;
    } catch (const spdlog::spdlog_ex &ex) {
        perror("spdlog init error.");
    }
}

void ZCLog::UnInit() {
    if (m_binit) {
        logxx_levelout(spdlog::level::level_enum::warn, "uninit");
        spdlog::drop_all();
        spdlog::shutdown();
        m_binit = false;
    }

    return;
}

int ZCLog::GetLogLevel() {
    return loggerPtr->level();
}

void ZCLog::SetLogLevel(int log_level) {
    if (log_level == spdlog::level::level_enum::off) {
        logxx_levelout(spdlog::level::level_enum::warn, "Given invalid log level {}", log_level);
    } else {
        loggerPtr->set_level((spdlog::level::level_enum)log_level);
        loggerPtr->flush_on((spdlog::level::level_enum)m_LogCfg.fulshlevel);
        m_LogCfg.level = log_level;
    }
}
