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

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <input/Input.h>
#include "ApplicationState.h"
#include "fs/WriteOnlyFileWithCache.h"
#include "fs/WUXFileWriter.h"

#define READ_NUM_SECTORS 128
#define WRITE_BUFFER_NUM_SECTORS 128
#define WUD_FILE_SIZE 0x5D3A00000L

class WUDDumperState : public ApplicationState {
public:

    enum eDumpTargetFormat {
        DUMP_AS_WUX,
        DUMP_AS_WUD,
    };

    enum eDumpState {
        STATE_ERROR,
        STATE_RETURN,
        STATE_OPEN_ODD1,
        STATE_PLEASE_INSERT_DISC,
        STATE_READ_DISC_INFO,
        STATE_READ_DISC_INFO_DONE,
        STATE_DUMP_DISC_KEY,
        STATE_DUMP_DISC_START,
        STATE_DUMP_DISC_DONE,
        STATE_WAIT_USER_ERROR_CONFIRM,
        STATE_DUMP_DISC,
    };

    enum eErrorState {
        ERROR_NONE,
        ERROR_READ_FIRST_SECTOR,
        ERROR_FILE_OPEN_FAILED,
        ERROR_MALLOC_FAILED,
        ERROR_NO_DISC_ID,
        ERROR_WRITE_FAILED,
        ERROR_NO_DISC_FOUND
    };

    explicit WUDDumperState(eDumpTargetFormat pTarget);

    ~WUDDumperState() override;

    eDumpState state;
    eDumpTargetFormat target;
    WUDDumperState::eErrorState errorState = ERROR_NONE;

    void render() override;

    eSubState update(Input *input) override;

    void setError(eErrorState err);

    [[nodiscard]] std::string ErrorMessage() const;

    [[nodiscard]] std::string ErrorDescription() const;

    void *sectorBuf = nullptr;

    int readResult = 0;
    int oddFd = -1;
    int retryCount = 10;

    OSTime startTime{};

    std::unique_ptr<WUDFileWriter> fileHandle = {};

    std::array<uint8_t, 11> discId{};

    uint32_t sectorBufSize{};

    //
    uint64_t currentSector{};
    uint64_t totalSectorCount{};

    //
    std::vector<uint64_t> skippedSectors;
    bool autoSkipOnError = false;

    // We need this to calculate the compression ratio.
    int32_t writtenSectors{};

    void *emptySector = nullptr;
};