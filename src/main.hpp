#pragma once

#include <android/log.h>
#include <cstdint>

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "EnchantLvlModifier", __VA_ARGS__)

class LevelController {
public:
    static int64_t getMinLevel(void* self);
    static int64_t getMaxLevel(void* self);
};

void HookLevels();