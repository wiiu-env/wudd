/****************************************************************************
 * Copyright (C) 2021 Maschell
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
#include "MainApplicationState.h"
#include "WUDDumperState.h"
#include "GMPartitionsDumperState.h"
#include <sysapp/launch.h>

MainApplicationState::MainApplicationState() {
    DEBUG_FUNCTION_LINE("Welcome!");
    this->state = STATE_WELCOME_SCREEN;
}

MainApplicationState::~MainApplicationState() = default;;

void MainApplicationState::render() {
    if (this->state == STATE_DO_SUBSTATE) {
        if (this->subState == nullptr) {
            OSFatal("SubState was null");
        }
        this->subState->render();
        return;
    }
    WiiUScreen::clearScreen();
    printHeader();

    if (this->state == STATE_WELCOME_SCREEN) {
        WiiUScreen::drawLine("Welcome to Wudump");
        WiiUScreen::drawLine("");
        WiiUScreen::drawLinef("%s Dump as WUX", this->selectedOption == 0 ? ">" : " ");
        WiiUScreen::drawLinef("%s Dump as WUD", this->selectedOption == 1 ? ">" : " ");
        WiiUScreen::drawLinef("%s Dump partition as .app", this->selectedOption == 2 ? ">" : " ");
        WiiUScreen::drawLinef("%s Exit", this->selectedOption == 3 ? ">" : " ");
    }

    printFooter();
    WiiUScreen::flipBuffers();
}

ApplicationState::eSubState MainApplicationState::update(Input *input) {
    if (this->state == STATE_WELCOME_SCREEN) {
        proccessMenuNavigation(input, 4);
        if (entrySelected(input)) {
            if (this->selectedOption == 0) {
                this->state = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<WUDDumperState>(WUDDumperState::DUMP_AS_WUX);
            } else if (this->selectedOption == 1) {
                this->state = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<WUDDumperState>(WUDDumperState::DUMP_AS_WUD);
            } else if (this->selectedOption == 2) {
                this->state = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<GMPartitionsDumperState>();
            } else {
                SYSLaunchMenu();
            }
            this->selectedOption = 0;
        }
    } else if (this->state == STATE_DO_SUBSTATE) {
        auto retSubState = this->subState->update(input);
        if (retSubState == SUBSTATE_RUNNING) {
            // keep running.
            return SUBSTATE_RUNNING;
        } else if (retSubState == SUBSTATE_RETURN) {
            DEBUG_FUNCTION_LINE("Delete sub state");
            this->subState.reset();
            this->state = STATE_WELCOME_SCREEN;
        }
    }
    return SUBSTATE_RUNNING;
}
