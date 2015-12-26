#ifndef LIBENG_LOG_H_
#define LIBENG_LOG_H_

#include <libeng/Global.h>

#define LOG_FORMAT(param)       "%s[%d]" param
#define LOG_LEVEL               5  // 4 level to add log (always log: LOGW, LOGE & LOGF)
                                    // Possible use: LOGI(LOG_LEVEL_MODULE + n, ... );
typedef enum {

#ifdef DEBUG
    LOG_UNKNOWN,    // UNKNOWN: Log at unexpected exception (such as unknown exception)
    LOG_DEFAULT,    // DEFAULT: Log unexpected default option (switch)
    LOG_VERBOSE,    // VERBOSE: Log at each funtion/method entries
    LOG_DEBUG,      // DEBUG: Log elapsed time
    LOG_INFO,       // INFO: Log infos in the code
    LOG_SILENT,     // SILENT: Shut!?!
#endif
    LOG_WARNING,    // WARNING: ...
    LOG_ERROR,      // ERROR: ...
    LOG_FATAL       // FATAL: ...

} LogType;

// mod: Modulo of value that increase to avoid too many logs: !(value % n) -> log

#ifdef __ANDROID__
#include <android/log.h>

#ifdef DEBUG
#define LOGU(level, mod, format, ...)       if ((level < LOG_LEVEL) && (!mod)) \
                                             { __android_log_print(ANDROID_LOG_UNKNOWN, eng::g_LogTag, format, __VA_ARGS__); }
#define LOGD(level, mod, format, ...)       if ((level < LOG_LEVEL) && (!mod)) \
                                             { __android_log_print(ANDROID_LOG_DEFAULT, eng::g_LogTag, format, __VA_ARGS__); }
#define LOGV(level, mod, format, ...)       if ((level < LOG_LEVEL) && (!mod)) \
                                             { __android_log_print(ANDROID_LOG_VERBOSE, eng::g_LogTag, format, __VA_ARGS__); }
#define LOGDE(level, mod, format, ...)      if ((level < LOG_LEVEL) && (!mod)) \
                                             { __android_log_print(ANDROID_LOG_DEBUG, eng::g_LogTag, format, __VA_ARGS__); }
#define LOGI(level, mod, format, ...)       if ((level < LOG_LEVEL) && (!mod)) \
                                             { __android_log_print(ANDROID_LOG_INFO, eng::g_LogTag, format, __VA_ARGS__); }
#define LOGS(level, mod, format, ...)       if ((level < LOG_LEVEL) && (!mod)) \
                                             { __android_log_print(ANDROID_LOG_SILENT, eng::g_LogTag, format, __VA_ARGS__); }

#else
#define LOGU(level, mod, format, ...)       ((void)0)
#define LOGD(level, mod, format, ...)       ((void)0)
#define LOGV(level, mod, format, ...)       ((void)0)
#define LOGDE(level, mod, format, ...)      ((void)0)
#define LOGI(level, mod, format, ...)       ((void)0)
#define LOGS(level, mod, format, ...)       ((void)0)

#endif
#define LOGW(format, ...)                   __android_log_print(ANDROID_LOG_WARN, eng::g_LogTag, format, __VA_ARGS__);
#define LOGE(format, ...)                   __android_log_print(ANDROID_LOG_ERROR, eng::g_LogTag, format, __VA_ARGS__);
#define LOGF(format, ...)                   __android_log_print(ANDROID_LOG_FATAL, eng::g_LogTag, format, __VA_ARGS__);

#else // !__ANDROID__

#ifdef DEBUG
extern "C" {

void LOGU(short level, unsigned int mod, const char* format, ...);
void LOGD(short level, unsigned int mod, const char* format, ...);
void LOGV(short level, unsigned int mod, const char* format, ...);
void LOGDE(short level, unsigned int mod, const char* format, ...);
void LOGI(short level, unsigned int mod, const char* format, ...);
void LOGS(short level, unsigned int mod, const char* format, ...);

} // extern "C"

#else
#define LOGU(level, mod, format, ...)       ((void)0)
#define LOGD(level, mod, format, ...)       ((void)0)
#define LOGV(level, mod, format, ...)       ((void)0)
#define LOGDE(level, mod, format, ...)      ((void)0)
#define LOGI(level, mod, format, ...)       ((void)0)
#define LOGS(level, mod, format, ...)       ((void)0)

#endif
extern "C" {

void LOGW(const char* format, ...);
void LOGE(const char* format, ...);
void LOGF(const char* format, ...);

} // extern "C"

#endif // __ANDROID__

#endif // LIBENG_LOG_H_
