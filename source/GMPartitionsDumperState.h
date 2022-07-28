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
#include "common/common.h"
#include "fs/WUXFileWriter.h"
#include "fs/WriteOnlyFileWithCache.h"
#include <WUD/NUSTitle.h>
#include <WUD/entities/TMD/Content.h>
#include <WUD/header/WiiUDiscHeader.h>
#include <input/Input.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class GMPartitionsDumperState : public ApplicationState {
public:
    enum eDumpState {
        STATE_ERROR,
        STATE_RETURN,
        STATE_OPEN_ODD1,
        STATE_READ_DISC_INFO,
        STATE_PLEASE_INSERT_DISC,
        STATE_READ_DISC_INFO_DONE,
        STATE_READ_COMMON_KEY,
        STATE_CREATE_DISC_READER,
        STATE_PARSE_DISC_HEADER,
        STATE_PROCESS_GM_PARTITIONS,
        STATE_CHOOSE_PARTITION_TO_DUMP,
        STATE_CREATE_DATA_PROVIDER,
        STATE_DUMP_PARTITION_TMD,
        STATE_DUMP_PARTITION_TICKET,
        STATE_DUMP_PARTITION_CERT,
        STATE_DUMP_PARTITION_CONTENTS,
        STATE_DUMP_DONE,
        STATE_ABORT_CONFIRMATION
    };

    enum eErrorState {
        ERROR_NONE,
        ERROR_MALLOC_FAILED,
        ERROR_READ_FIRST_SECTOR,
        ERROR_OPEN_ODD1,
        ERROR_PARSE_DISC_HEADER,
        ERROR_NO_GM_PARTITION,
        ERROR_CREATE_DIR,
        ERROR_FAILED_TO_GET_NUSTITLE,
        ERROR_FAILED_WRITE_TMD,
        ERROR_FAILED_WRITE_TICKET,
        ERROR_FAILED_WRITE_CERT,
        ERROR_FIND_CONTENT_BY_INDEX,
        ERROR_FAILED_CREATE_FILE,
        ERROR_FAILED_WRITE_H3,
        ERROR_READ_CONTENT,
        ERROR_WRITE_CONTENT
    };

    explicit GMPartitionsDumperState(eDumpTarget pTargetDevice);

    ~GMPartitionsDumperState() override;

    eDumpState state;
    eErrorState errorState = ERROR_NONE;

    void render() override;

    eSubState update(Input *input) override;

    void setError(eErrorState err);

    [[nodiscard]] std::string getPathForDevice(eDumpTarget target) const;

    std::array<uint8_t, 11> discId{};
    std::array<uint8_t, 0x10> cKey{};

    int32_t oddFd   = -1;
    void *sectorBuf = nullptr;
    uint32_t sectorBufSize;
    std::shared_ptr<DiscReader> discReader;
    std::unique_ptr<WiiUDiscHeader> discHeader;

    std::shared_ptr<WiiUGMPartition> curPartition = nullptr;
    std::shared_ptr<NUSDataProvider> dataProvider = nullptr;
    std::string targetPath;
    std::shared_ptr<NUSTitle> curNUSTitle = nullptr;

    uint16_t curContentIndex            = 0;
    std::unique_ptr<CFile> contentFile  = nullptr;
    std::shared_ptr<Content> curContent = nullptr;
    uint8_t *readBuffer                 = nullptr;
    uint32_t curOffsetInContent         = 0;

    [[nodiscard]] std::string ErrorMessage() const;

    [[nodiscard]] std::string ErrorDescription() const;

    std::vector<std::pair<std::shared_ptr<WiiUGMPartition>, std::shared_ptr<NUSTitle>>> gmPartitionPairs;

    eDumpTarget targetDevice = TARGET_SD;

    std::string getPathNameForDisc();

    OSTime dumpStartDate;
};