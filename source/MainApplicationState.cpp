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
#include "GMPartitionsDumperState.h"
#include "WUDDumperState.h"
#include <sysapp/launch.h>

MainApplicationState::MainApplicationState() {
    DEBUG_FUNCTION_LINE("Welcome!");
    this->state = STATE_WELCOME_SCREEN;
}

MainApplicationState::~MainApplicationState() = default;
;

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
        WiiUScreen::drawLinef("%s Dump as WUX", this->selectedOptionY == 0 ? ">" : " ");
        WiiUScreen::drawLinef("%s Dump as WUD", this->selectedOptionY == 1 ? ">" : " ");
        WiiUScreen::drawLinef("%s Dump partition as .app", this->selectedOptionY == 2 ? ">" : " ");
        WiiUScreen::drawLine();
        WiiUScreen::drawLinef("%s Dumptarget:", this->selectedOptionY == 3 ? ">" : " ");
        if (ntfs_mount_count > 0) {
            WiiUScreen::drawLinef("     [%s] SD    [%s] NTFS (USB)", dumpTarget == TARGET_SD ? "x" : " ", dumpTarget == TARGET_NTFS ? "x" : " ");
        } else {
            WiiUScreen::drawLinef("     [%s] SD    ???  NTFS (USB) (not connected)", dumpTarget == TARGET_SD ? "*" : " ");
        }
        WiiUScreen::drawLine();
        if (!gRunFromHBL) {
            WiiUScreen::drawLinef("%s Exit", this->selectedOptionY == 4 ? ">" : " ");
        }
    }

    printFooter();
    WiiUScreen::flipBuffers();
}

ApplicationState::eSubState MainApplicationState::update(Input *input) {
    if (this->state == STATE_WELCOME_SCREEN) {
        int optionCount = gRunFromHBL ? 4 : 5;
        proccessMenuNavigationY(input, optionCount);
        if (selectedOptionY == 3) {
            if (ntfs_mount_count > 0) {
                proccessMenuNavigationX(input, 2);
                if (selectedOptionX == 0) {
                    dumpTarget = TARGET_SD;
                } else {
                    dumpTarget = TARGET_NTFS;
                }
            }
        }
        if (entrySelected(input)) {
            if (this->selectedOptionY == 0) {
                this->state    = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<WUDDumperState>(WUDDumperState::DUMP_AS_WUX, dumpTarget);
            } else if (this->selectedOptionY == 1) {
                this->state    = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<WUDDumperState>(WUDDumperState::DUMP_AS_WUD, dumpTarget);
            } else if (this->selectedOptionY == 2) {
                this->state    = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<GMPartitionsDumperState>(dumpTarget);
            } else if (this->selectedOptionY == 3) {
                //
            } else {
                if (!gRunFromHBL) {
                    SYSLaunchMenu();
                }
            }
            this->selectedOptionY = 0;
        }
    } else if (this->state == STATE_DO_SUBSTATE) {
        auto retSubState = this->subState->update(input);
        if (retSubState == SUBSTATE_RUNNING) {
            // keep running.
            return SUBSTATE_RUNNING;
        } else if (retSubState == SUBSTATE_RETURN) {
            this->subState.reset();
            this->state = STATE_WELCOME_SCREEN;
        }
    }
    return SUBSTATE_RUNNING;
}
