// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __ZC_LOG_H__
#define __ZC_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#if ZC_DEBUG
#define ZC_LOG_CONS_DEF_LEVEL LEVEL_DEBUG
#define ZC_LOG_DEF_LEVEL LEVEL_DEBUG
#else
#define ZC_LOG_CONS_DEF_LEVEL LEVEL_INFO
#define ZC_LOG_DEF_LEVEL LEVEL_DEBUG
#endif

enum LogLevel {
    LEVEL_TRACE = 0,
    LEVEL_DEBUG = 1,
    LEVEL_INFO = 2,
    LEVEL_WARN = 3,
    LEVEL_ERROR = 4,
    LEVEL_CRITI = 5,

    LEVEL_BUTT = 6,
};

#ifndef __FILENAME__
// #warning "nodefine __FILENAME__"
#define __FILENAME__ __FILE__
#endif

#define LOG_TRACE(fmt, ...) \
    zc_log_levelout(LEVEL_TRACE, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) \
    zc_log_levelout(LEVEL_DEBUG, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
// #define LOG_TRACE(fmt, ...) zc_log_levelout(LEVEL_INFO, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__,
// ##__VA_ARGS__) #define LOG_DEBUG(fmt, ...) zc_log_levelout(LEVEL_INFO, "[%s %d][%s]" fmt, __FILENAME__, __LINE__,
// __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    zc_log_levelout(LEVEL_INFO, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    zc_log_levelout(LEVEL_WARN, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
    zc_log_levelout(LEVEL_ERROR, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_CRITI(fmt, ...) \
    zc_log_levelout(LEVEL_CRITI, "[%s %d][%s]" fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

void zc_log_levelout(int level, const char *fmt, ...);
int zc_log_init(const char *szName);
int zc_log_uninit();

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__ZC_LOG_H__*/
