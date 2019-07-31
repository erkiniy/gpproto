//
// Created by ProDigital on 7/27/18.
//

#ifndef GPPROTO_LOGGING_H
#define GPPROTO_LOGGING_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <stdio.h>
#include <mutex>

void gp_log_file_set_path(const char* path);
void gp_log_file_printf(char level, const char* msg, ...);
void gp_log_file_write_header(FILE* file);
void gp_log_printf(char level, const char* msg, ...);

static std::mutex log_mtx;

#ifdef __ANDROID__
#include <android/log.h>
#define _LOG_WRAP(...) __VA_ARGS__

#define TAG "gpproto-native"

#define LOGV(...) { _android_log_print(ANDROID_LOG_VERBOSE, TAG, _LOG_WRAP(__VA_ARGS__)); gp_log_file_printf('V', __VA_ARGS__);}
#define LOGD(...) { _android_log_print(ANDROID_LOG_DEBUG, TAG, _LOG_WRAP(__VA_ARGS__)); gp_log_file_printf('D', __VA_ARGS__);}
#define LOGI(...) { _android_log_print(ANDROID_LOG_INFO, TAG, _LOG_WRAP(__VA_ARGS__)); gp_log_file_printf('I', __VA_ARGS__);}
#define LOGW(...) { _android_log_print(ANDROID_LOG_WARN, TAG, _LOG_WRAP(__VA_ARGS__)); gp_log_file_printf('W', __VA_ARGS__);}
#define LOGE(...) { _android_log_print(ANDROID_LOG_ERROR, TAG, _LOG_WRAP(__VA_ARGS__)); gp_log_file_printf('E', __VA_ARGS__);}

#elif defined(__APPLE__) && TARGET_OS_IPHONE && defined(GPPRTO_HAVE_LOG)
#include "os/darwin/GPLogWrapperDarwin.h"

#define LOGV(...) { __gpproto_call_gplog("V/gpproto: " msg, ##__VA_ARGS__); gp_log_file_printf('V', msg, ##__VA_ARGS__);}
#define LOGD(...) { __gpproto_call_gplog("D/gpproto: " msg, ##__VA_ARGS__); gp_log_file_printf('D', msg, ##__VA_ARGS__);}
#define LOGI(...) { __gpproto_call_gplog("I/gpproto: " msg, ##__VA_ARGS__); gp_log_file_printf('I', msg, ##__VA_ARGS__);}
#define LOGW(...) { __gpproto_call_gplog("W/gpproto: " msg, ##__VA_ARGS__); gp_log_file_printf('W', msg, ##__VA_ARGS__);}
#define LOGE(...) { __gpproto_call_gplog("E/gpproto: " msg, ##__VA_ARGS__); gp_log_file_printf('E', msg, ##__VA_ARGS__);}

#elif defined(_WIN32) && defined(_DEBUG)

#include <windows.h>
#include <stdio.h>

#define _GP_W32_LOG_PRINT(verb, msg ...) { char __log_buf[1024]; snprintf(__log_buf, 1024, "%c/gpproto: " msg "\n", verb, ##__VA_ARGS__); OutputDebugStringA(__log_buf); gp_log_file_printf((char)verb, msg, __VA_ARGS__);}

#define LOGV(msg, ...) _GP_W32_LOG_PRINT('V', msg, ##__VA_ARGS__)
#define LOGD(msg, ...) _GP_W32_LOG_PRINT('D', msg, ##__VA_ARGS__)
#define LOGI(msg, ...) _GP_W32_LOG_PRINT('I', msg, ##__VA_ARGS__)
#define LOGW(msg, ...) _GP_W32_LOG_PRINT('W', msg, ##__VA_ARGS__)
#define LOGE(msg, ...) _GP_W32_LOG_PRINT('E', msg, ##__VA_ARGS__)

#else

#include <stdio.h>

#define GP_LOG_PRINT(verb, msg, ...) { log_mtx.lock(); gp_log_printf(verb, msg, ##__VA_ARGS__); gp_log_file_printf(verb, msg, ##__VA_ARGS__); log_mtx.unlock(); }

#define LOGV(msg, ...) GP_LOG_PRINT('V', msg, ##__VA_ARGS__)
#define LOGD(msg, ...) GP_LOG_PRINT('D', msg, ##__VA_ARGS__)
#define LOGI(msg, ...) GP_LOG_PRINT('I', msg, ##__VA_ARGS__)
#define LOGW(msg, ...) GP_LOG_PRINT('W', msg, ##__VA_ARGS__)
#define LOGE(msg, ...) GP_LOG_PRINT('E', msg, ##__VA_ARGS__)

#endif

#if !defined(snprintf) && defined(_WIN32) && defined(__cplusplus_winrt)
#define snprintf _snprintf
#endif

#ifdef GPPROTO_LOG_VERBOSITY
#if GPPROTO_LOG_VERBOSITY < 5
#undef LOGV
#define LOGV(msg, ...)
#endif
#if GPPROTO_LOG_VERBOSITY < 4
#undef LOGD
#define LOGD(msg, ...)
#endif
#if GPPROTO_LOG_VERBOSITY < 3
#undef LOGI
#define LOGI(msg, ...)
#endif
#if GPPROTO_LOG_VERBOSITY < 2
#undef LOGW
#define LOGW(msg, ...)
#endif
#if GPPROTO_LOG_VERBOSITY < 1
#undef LOGE
#define LOGE(msg, ...)
#endif
#endif

#endif //GPPROTO_LOGGING_H
