#pragma once
/****************************************************************************
 * Copyright (C) 2015 Dimok
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

#include "Input.h"
#include <padscore/kpad.h>
#include <padscore/wpad.h>

class WPADInput : public Input {
public:
    WPADInput(KPADChan channel) {
        this->channel = channel;
    }

    ~WPADInput() override {}

    uint32_t remapWiiMoteButtons(uint32_t buttons) {
        uint32_t conv_buttons = 0;

        if (buttons & WPAD_BUTTON_LEFT)
            conv_buttons |= Input::BUTTON_LEFT;

        if (buttons & WPAD_BUTTON_RIGHT)
            conv_buttons |= Input::BUTTON_RIGHT;

        if (buttons & WPAD_BUTTON_DOWN)
            conv_buttons |= Input::BUTTON_DOWN;

        if (buttons & WPAD_BUTTON_UP)
            conv_buttons |= Input::BUTTON_UP;

        if (buttons & WPAD_BUTTON_PLUS)
            conv_buttons |= Input::BUTTON_PLUS;

        if (buttons & WPAD_BUTTON_2)
            conv_buttons |= Input::BUTTON_2;

        if (buttons & WPAD_BUTTON_1)
            conv_buttons |= Input::BUTTON_1;

        if (buttons & WPAD_BUTTON_B)
            conv_buttons |= Input::BUTTON_B;

        if (buttons & WPAD_BUTTON_A)
            conv_buttons |= Input::BUTTON_A;

        if (buttons & WPAD_BUTTON_MINUS)
            conv_buttons |= Input::BUTTON_MINUS;

        if (buttons & WPAD_BUTTON_Z)
            conv_buttons |= Input::BUTTON_Z;

        if (buttons & WPAD_BUTTON_C)
            conv_buttons |= Input::BUTTON_C;

        if (buttons & WPAD_BUTTON_HOME)
            conv_buttons |= Input::BUTTON_HOME;

        return conv_buttons;
    }

    uint32_t remapClassicButtons(uint32_t buttons) {
        uint32_t conv_buttons = 0;

        if (buttons & WPAD_CLASSIC_BUTTON_LEFT)
            conv_buttons |= Input::BUTTON_LEFT;

        if (buttons & WPAD_CLASSIC_BUTTON_RIGHT)
            conv_buttons |= Input::BUTTON_RIGHT;

        if (buttons & WPAD_CLASSIC_BUTTON_DOWN)
            conv_buttons |= Input::BUTTON_DOWN;

        if (buttons & WPAD_CLASSIC_BUTTON_UP)
            conv_buttons |= Input::BUTTON_UP;

        if (buttons & WPAD_CLASSIC_BUTTON_PLUS)
            conv_buttons |= Input::BUTTON_PLUS;

        if (buttons & WPAD_CLASSIC_BUTTON_X)
            conv_buttons |= Input::BUTTON_X;

        if (buttons & WPAD_CLASSIC_BUTTON_Y)
            conv_buttons |= Input::BUTTON_Y;

        if (buttons & WPAD_CLASSIC_BUTTON_B)
            conv_buttons |= Input::BUTTON_B;

        if (buttons & WPAD_CLASSIC_BUTTON_A)
            conv_buttons |= Input::BUTTON_A;

        if (buttons & WPAD_CLASSIC_BUTTON_MINUS)
            conv_buttons |= Input::BUTTON_MINUS;

        if (buttons & WPAD_CLASSIC_BUTTON_HOME)
            conv_buttons |= Input::BUTTON_HOME;

        if (buttons & WPAD_CLASSIC_BUTTON_ZR)
            conv_buttons |= Input::BUTTON_ZR;

        if (buttons & WPAD_CLASSIC_BUTTON_ZL)
            conv_buttons |= Input::BUTTON_ZL;

        if (buttons & WPAD_CLASSIC_BUTTON_R)
            conv_buttons |= Input::BUTTON_R;

        if (buttons & WPAD_CLASSIC_BUTTON_L)
            conv_buttons |= Input::BUTTON_L;

        return conv_buttons;
    }

    bool update(int32_t width, int32_t height) {
        lastData = data;
        WPADExtensionType type;
        if (WPADProbe(channel, &type) != 0) {
            data.buttons_h = 0;
            return false;
        }

        KPADRead(channel, &kpad, 1);

        if (kpad.extensionType == WPAD_EXT_CORE || kpad.extensionType == WPAD_EXT_NUNCHUK) {
            data.buttons_r = remapWiiMoteButtons(kpad.release);
            data.buttons_h = remapWiiMoteButtons(kpad.hold);
            data.buttons_d = remapWiiMoteButtons(kpad.trigger);
        } else {
            data.buttons_r = remapClassicButtons(kpad.classic.release);
            data.buttons_h = remapClassicButtons(kpad.classic.hold);
            data.buttons_d = remapClassicButtons(kpad.classic.trigger);
        }

        data.validPointer = (kpad.posValid == 1 || kpad.posValid == 2) &&
                            (kpad.pos.x >= -1.0f && kpad.pos.x <= 1.0f) &&
                            (kpad.pos.y >= -1.0f && kpad.pos.y <= 1.0f);

        //! calculate the screen offsets if pointer is valid else leave old value
        if (data.validPointer) {
            data.x = (width >> 1) * kpad.pos.x;
            data.y = (height >> 1) * (-kpad.pos.y);

            if (kpad.angle.y > 0.0f) {
                data.pointerAngle = (-kpad.angle.x + 1.0f) * 0.5f * 180.0f;
            } else {
                data.pointerAngle = (kpad.angle.x + 1.0f) * 0.5f * 180.0f - 180.0f;
            }
        }

        return true;
    }

    static void init() {
        KPADInit();
        WPADEnableURCC(1);
    }

    static void close() {
    }

private:
    KPADStatus kpad{};
    KPADChan channel;
};