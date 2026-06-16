//
// Created by cams on 6/16/26.
//

#ifndef GOTCHA_LOG_H
#define GOTCHA_LOG_H

#include <android/log.h>

#define LOG_TAG "[Gotcha]"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#endif //GOTCHA_LOG_H
