#include "WiiUScreen.h"
#include "ScreenUtils.h"
#include <coreinit/memdefaultheap.h>
#include <cstdarg>
#include <cstdio>

void *WiiUScreen::sBufferTV            = nullptr;
void *WiiUScreen::sBufferDRC           = nullptr;
uint32_t WiiUScreen::sBufferSizeTV     = 0;
uint32_t WiiUScreen::sBufferSizeDRC    = 0;
bool WiiUScreen::sConsoleHasForeground = true;
uint32_t WiiUScreen::consoleColor      = 0x041F60FF;
uint32_t WiiUScreen::consoleCursorY    = 0;


uint32_t WiiUScreen::ProcCallbackAcquired([[maybe_unused]] void *context) {
    MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    if (sBufferSizeTV) {
        sBufferTV = MEMAllocFromFrmHeapEx(heap, sBufferSizeTV, 4);
    }

    if (sBufferSizeDRC) {
        sBufferDRC = MEMAllocFromFrmHeapEx(heap, sBufferSizeDRC, 4);
    }

    sConsoleHasForeground = true;
    OSScreenSetBufferEx(SCREEN_TV, sBufferTV);
    OSScreenSetBufferEx(SCREEN_DRC, sBufferDRC);
    return 0;
}

uint32_t WiiUScreen::ProcCallbackReleased([[maybe_unused]] void *context) {
    MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    MEMFreeByStateToFrmHeap(heap, CONSOLE_FRAME_HEAP_TAG);
    sConsoleHasForeground = FALSE;
    return 0;
}

bool WiiUScreen::Init() {
    OSScreenInit();
    sBufferSizeTV  = OSScreenGetBufferSizeEx(SCREEN_TV);
    sBufferSizeDRC = OSScreenGetBufferSizeEx(SCREEN_DRC);

    WiiUScreen::ProcCallbackAcquired(nullptr);
    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);

    ScreenUtils::OSScreenClear(CONSOLE_SCREEN_BOTH, WiiUScreen::consoleColor);

    ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, WiiUScreen::ProcCallbackAcquired, nullptr, 100);
    ProcUIRegisterCallback(PROCUI_CALLBACK_RELEASE, WiiUScreen::ProcCallbackReleased, nullptr, 100);
    return true;
}

void WiiUScreen::DeInit() {
    if (sConsoleHasForeground) {
        OSScreenShutdown();
        WiiUScreen::ProcCallbackReleased(nullptr);
    }
}

void WiiUScreen::drawLinef(const char *fmt, ...) {
    char *buf = (char *) MEMAllocFromDefaultHeapEx(PRINTF_BUFFER_LENGTH, 4);
    va_list va;

    if (!buf) {
        return;
    }

    va_start(va, fmt);
    vsnprintf(buf, PRINTF_BUFFER_LENGTH, fmt, va);

    WiiUScreen::drawLine(buf);

    MEMFreeToDefaultHeap(buf);
    va_end(va);
}

void WiiUScreen::drawLine() {
    WiiUScreen::drawLine("");
}

void WiiUScreen::drawLine(const char *msg) {
    ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_BOTH, 0, consoleCursorY++, msg);
}

void WiiUScreen::flipBuffers() {
    ScreenUtils::flipBuffers(CONSOLE_SCREEN_BOTH);
}

void WiiUScreen::clearScreen() {
    ScreenUtils::OSScreenClear(CONSOLE_SCREEN_BOTH, WiiUScreen::consoleColor);
    consoleCursorY = 0;
}
