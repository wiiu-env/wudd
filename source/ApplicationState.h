#pragma once

#include "common/common.h"
#include "input/Input.h"
#include "utils/ScreenUtils.h"
#include "utils/WiiUScreen.h"
#include "version.h"
#include <stdint.h>

class ApplicationState {
public:
    enum eSubState {
        SUBSTATE_RUNNING,
        SUBSTATE_RETURN,
    };

    virtual ~ApplicationState() = default;

    virtual void render() = 0;

    virtual eSubState update(Input *input) = 0;

    virtual void proccessMenuNavigationY(Input *input, int32_t maxOptionValue) {
        if (input->data.buttons_d & Input::BUTTON_UP) {
            this->selectedOptionY--;
        } else if (input->data.buttons_d & Input::BUTTON_DOWN) {
            this->selectedOptionY++;
        }
        if (this->selectedOptionY < 0) {
            this->selectedOptionY = maxOptionValue - 1;
        } else if (this->selectedOptionY >= maxOptionValue) {
            this->selectedOptionY = 0;
        }
    }

    virtual void proccessMenuNavigationX(Input *input, int32_t maxOptionValue) {
        if (input->data.buttons_d & Input::BUTTON_LEFT) {
            this->selectedOptionX--;
        } else if (input->data.buttons_d & Input::BUTTON_RIGHT) {
            this->selectedOptionX++;
        }

        if (this->selectedOptionX < 0) {
            this->selectedOptionX = maxOptionValue - 1;
        } else if (this->selectedOptionX >= maxOptionValue) {
            this->selectedOptionX = 0;
        }
    }

    virtual bool buttonPressed(Input *input, Input::eButtons button) {
        return input->data.buttons_d & button;
    }

    virtual bool entrySelected(Input *input) {
        return input->data.buttons_d & Input::BUTTON_A;
    }

    virtual void printHeader() {
        WiiUScreen::drawLine("WUDD - Wii U Disc Dumper v1.2.2" VERSION_EXTRA);
        WiiUScreen::drawLine("==================");
        WiiUScreen::drawLine("");
    }

    virtual void printFooter() {
        if (gRunFromHBL && !gBlockHomeButton) {
            ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_TV, 0, 25, "Press HOME to exit to HBL");
            ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_DRC, 0, 15, "Press HOME to exit to HBL");
        } else if (gRunFromHBL && gBlockHomeButtonCooldown > 0) {
            ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_TV, 0, 25, "You can not exit while dumping.");
            ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_DRC, 0, 15, "You can not exit while dumping.");
            gBlockHomeButtonCooldown--;
        }
        ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_TV, 0, 27, "Created by Maschell, inspired by wudump from FIX94");
        ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_DRC, 0, 17, "Created by Maschell, inspired by wudump from FIX94");
    }


    int selectedOptionY = 0;
    int selectedOptionX = 0;
};
