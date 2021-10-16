#pragma once


#include <utils/WiiUScreen.h>
#include <utils/ScreenUtils.h>

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
            this->selectedOptionY = maxOptionValue;
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
            this->selectedOptionX = maxOptionValue;
        } else if (this->selectedOptionX >= maxOptionValue) {
            this->selectedOptionX = 0;
        }
    }

    virtual bool entrySelected(Input *input) {
        return input->data.buttons_d & Input::BUTTON_A;
    }

    virtual void printHeader() {
        WiiUScreen::drawLine("WUDD - Wii U Disc Dumper");
        WiiUScreen::drawLine("==================");
        WiiUScreen::drawLine("");
    }

    virtual void printFooter() {
        ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_TV, 0, 27, "Created by Maschell, inspired by wudump from FIX94");
        ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_DRC, 0, 17, "Created by Maschell, inspired by wudump from FIX94");
    }


    int selectedOptionY = 0;
    int selectedOptionX = 0;
};