#include "Log.h"
#import <Foundation/Foundation.h>

void LOG(LogType type, const char* format, va_list args) {
    @autoreleasepool {

        NSMutableString* msg = [[NSMutableString alloc] init];
        switch (type) {
#ifdef DEBUG
            case LOG_UNKNOWN: [msg appendString:@"UNKNOWN: "]; break;
            case LOG_DEFAULT: [msg appendString:@"DEFAULT: "]; break;
            case LOG_VERBOSE: [msg appendString:@"VERBOSE: "]; break;
            case LOG_DEBUG:   [msg appendString:@"DEBUG: "]; break;
            case LOG_INFO:    [msg appendString:@"INFO: "]; break;
            case LOG_SILENT:  [msg appendString:@"SILENT: "]; break;
#endif
            case LOG_WARNING: [msg appendString:@"WARNING: "]; break;
            case LOG_ERROR:   [msg appendString:@"ERROR: "]; break;
            case LOG_FATAL:   [msg appendString:@"FATAL: "]; break;
        }
        [msg appendString:[[NSString alloc] initWithBytes:format length:strlen(format) encoding:NSASCIIStringEncoding]];
        NSLogv(msg, args);
    }
}

//////
#ifdef DEBUG
void LOGU(short level, unsigned int mod, const char* format, ...) {
    if ((level < LOG_LEVEL) && (!mod)) { va_list args; va_start(args, format); LOG(LOG_UNKNOWN, format, args); va_end(args); }
}
void LOGD(short level, unsigned int mod, const char* format, ...) {
    if ((level < LOG_LEVEL) && (!mod)) { va_list args; va_start(args, format); LOG(LOG_DEFAULT, format, args); va_end(args); }
}
void LOGV(short level, unsigned int mod, const char* format, ...) {
    if ((level < LOG_LEVEL) && (!mod)) { va_list args; va_start(args, format); LOG(LOG_VERBOSE, format, args); va_end(args); }
}
void LOGDE(short level, unsigned int mod, const char* format, ...) {
    if ((level < LOG_LEVEL) && (!mod)) { va_list args; va_start(args, format); LOG(LOG_DEBUG, format, args); va_end(args); }
}
void LOGI(short level, unsigned int mod, const char* format, ...) {
    if ((level < LOG_LEVEL) && (!mod)) { va_list args; va_start(args, format); LOG(LOG_INFO, format, args); va_end(args); }
}
void LOGS(short level, unsigned int mod, const char* format, ...) {
    if ((level < LOG_LEVEL) && (!mod)) { va_list args; va_start(args, format); LOG(LOG_SILENT, format, args); va_end(args); }
}
#endif
void LOGW(const char* format, ...) { va_list args; va_start(args, format); LOG(LOG_WARNING, format, args); va_end(args); }
void LOGE(const char* format, ...) { va_list args; va_start(args, format); LOG(LOG_ERROR, format, args); va_end(args); }
void LOGF(const char* format, ...) { va_list args; va_start(args, format); LOG(LOG_FATAL, format, args); va_end(args); }
