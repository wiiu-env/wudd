#pragma once

#include <whb/log.h>
#include <whb/log_console.h>

#include "utils/logger.h"
#include <coreinit/cache.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/memheap.h>
#include <coreinit/memory.h>
#include <coreinit/screen.h>
#include <cstring>
#include <proc_ui/procui.h>

#define NUM_LINES              (16)
#define LINE_LENGTH            (128)
#define CONSOLE_FRAME_HEAP_TAG (0x000DECAF)
#define PRINTF_BUFFER_LENGTH   2048

#define DEBUG_CONSOLE_LOG(FMT, ARGS...)     \
    do {                                    \
        DEBUG_FUNCTION_LINE(FMT, ##ARGS);   \
        WiiUScreen::drawLinef(FMT, ##ARGS); \
    } while (0)


class WiiUScreen {

public:
    static uint32_t ProcCallbackAcquired([[maybe_unused]] void *context);

    static uint32_t ProcCallbackReleased([[maybe_unused]] void *context);

    static bool Init();

    static void DeInit();

    static void drawLinef(const char *fmt, ...);

    static void drawLine(const char *fmt);

    static void drawLine();

    static void flipBuffers();

    static void clearScreen();

private:
    static void *sBufferTV, *sBufferDRC;
    static uint32_t sBufferSizeTV, sBufferSizeDRC;
    static bool sConsoleHasForeground;
    static uint32_t consoleColor;
    static uint32_t consoleCursorY;
};