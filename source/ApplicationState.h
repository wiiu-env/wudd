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

    virtual void proccessMenuNavigation(Input *input, int32_t maxOptionValue) {
        if (input->data.buttons_d & Input::BUTTON_UP) {
            this->selectedOption--;
        } else if (input->data.buttons_d & Input::BUTTON_DOWN) {
            this->selectedOption++;
        }
        if (this->selectedOption < 0) {
            this->selectedOption = maxOptionValue;
        } else if (this->selectedOption >= maxOptionValue) {
            this->selectedOption = 0;
        }
    }

    virtual bool entrySelected(Input *input) {
        return input->data.buttons_d & Input::BUTTON_A;
    }

    virtual void printHeader() {
        WiiUScreen::drawLine("Wudump");
        WiiUScreen::drawLine("==================");
        WiiUScreen::drawLine("");
    }

    virtual void printFooter() {
        ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_TV, 0, 27, "By Maschell");
        ScreenUtils::printTextOnScreen(CONSOLE_SCREEN_DRC, 0, 17, "By Maschell");
    }


    int selectedOption = 0;
};