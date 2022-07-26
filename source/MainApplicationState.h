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
#pragma once

#include "ApplicationState.h"
#include "fs/CFile.hpp"
#include "input/Input.h"
#include <common/common.h>
#include <ctime>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <string>


class MainApplicationState : public ApplicationState {
public:
    enum eGameState {
        STATE_WELCOME_SCREEN,
        STATE_DO_SUBSTATE,
    };

    MainApplicationState();

    ~MainApplicationState() override;

    void render() override;

    ApplicationState::eSubState update(Input *input) override;

private:
    std::unique_ptr<ApplicationState> subState{};

    eGameState state = STATE_WELCOME_SCREEN;

    eDumpTarget dumpTarget = TARGET_SD;
};