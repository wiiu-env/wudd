/****************************************************************************
 * Copyright (C) 2018-2020 Maschell
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
#pragma once

#include <stdint.h>

//! Defines the ID of a display usable with OSScreen.
typedef enum ConsoleScreenID {
    //! Represents the TV connected to the system.
    CONSOLE_SCREEN_TV = 0,
    //! Represents the screen in the DRC (gamepad).
    CONSOLE_SCREEN_DRC = 1,
    //! Represents both screens
    CONSOLE_SCREEN_BOTH = 2,
} ConsoleScreenID;

class ScreenUtils {
public:
    /**
        Clears the screen for the given screens
        \param screen defines on which screens should be printed
        \param x defines the x position (character position) where the text should be printed
        \param y defines on which line the text should be printed
        \param msg C string that contains the text to be printed.
    **/
    static void printTextOnScreen(ConsoleScreenID screen, uint32_t x, uint32_t y, const char *msg);

    /**
        Clears the screen for the given screens
        \param screen defines which screens should be cleared
    **/
    static void OSScreenClear(ConsoleScreenID screen, uint32_t i);

    /**
        Flips the buffer for the given screens
        \param screen defines which screens should be flipped.
    **/
    static void flipBuffers(ConsoleScreenID screen);

private:
    ScreenUtils() = default;

    ~ScreenUtils() = default;
};