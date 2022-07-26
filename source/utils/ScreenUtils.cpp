/****************************************************************************
 * Copyright (C) 2018 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "ScreenUtils.h"
#include <coreinit/screen.h>

void ScreenUtils::printTextOnScreen(ConsoleScreenID screen, uint32_t x, uint32_t y, const char *msg) {
    if (!msg) {
        return;
    }
    if (screen != CONSOLE_SCREEN_DRC) { // Draw TV if it's not DRC exclusive.
        OSScreenPutFontEx(SCREEN_TV, x, y, msg);
    }
    if (screen != CONSOLE_SCREEN_TV) { // Draw DRC if it's not TV exclusive.
        OSScreenPutFontEx(SCREEN_DRC, x, y, msg);
    }
}

void ScreenUtils::OSScreenClear(ConsoleScreenID screen, uint32_t color) {
    if (screen != CONSOLE_SCREEN_DRC) { // Clear TV if it's not DRC exclusive.
        OSScreenClearBufferEx(SCREEN_TV, color);
    }
    if (screen != CONSOLE_SCREEN_TV) { // Clear DRC if it's not TV exclusive.
        OSScreenClearBufferEx(SCREEN_DRC, color);
    }
}

void ScreenUtils::flipBuffers(ConsoleScreenID screen) {
    if (screen != CONSOLE_SCREEN_DRC) { // Flip TV buffer if it's not DRC exclusive.
        OSScreenFlipBuffersEx(SCREEN_TV);
    }
    if (screen != CONSOLE_SCREEN_TV) { // Flip DRC buffer if it's not TV exclusive.
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }
}
