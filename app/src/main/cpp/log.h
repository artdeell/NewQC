//
// Created by maks on 03.12.2024.
//

#ifndef NEWQC_LOG_H
#define NEWQC_LOG_H

#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#endif //NEWQC_LOG_H
