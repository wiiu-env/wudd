#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <cstring>
#include <whb/log.h>

#define __FILENAME_X__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILENAME__   (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILENAME_X__)

#define DEBUG_FUNCTION_LINE(FMT, ARGS...)                                                     \
    do {                                                                                      \
        WHBLogPrintf("[%23s]%30s@L%04d: " FMT, __FILENAME__, __FUNCTION__, __LINE__, ##ARGS); \
    } while (0)

#define DEBUG_FUNCTION_LINE_WRITE(FMT, ARGS...)                                                  \
    do {                                                                                         \
        WHBLogWritef("[%23s]%30s@L%04d: " FMT "", __FILENAME__, __FUNCTION__, __LINE__, ##ARGS); \
    } while (0)

#ifdef __cplusplus
}
#endif