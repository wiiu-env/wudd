/****************************************************************************
 * Copyright (C) 2016-2021 Maschell
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

#include "WiiUPartition.h"
#include <WUD/DiscReader.h>
#include <WUD/entities/FST/FST.h>
#include <cstdint>
#include <memory>
#include <utils/blocksize/AddressInDiscBlocks.h>
#include <utils/blocksize/DiscBlockSize.h>

#define WUD_TMD_FILENAME    "title.tmd"
#define WUD_TICKET_FILENAME "title.tik"
#define WUD_CERT_FILENAME   "title.cert"

class WiiUPartitions {

public:
    static bool getFSTEntryAsByte(std::string &filePath,
                                  const std::shared_ptr<FST> &fst,
                                  const AddressInDiscBlocks &volumeAddress,
                                  std::shared_ptr<DiscReader> &discReader,
                                  std::vector<uint8_t> &out_data);

    std::vector<std::shared_ptr<WiiUPartition>> partitions;
    static constexpr uint32_t LENGTH = 30720;

    static std::optional<std::unique_ptr<WiiUPartitions>> make_unique(
            std::shared_ptr<DiscReader> &discReader,
            uint32_t offset,
            uint32_t numberOfPartitions,
            const DiscBlockSize &blockSize);

private:
    explicit WiiUPartitions(std::vector<std::shared_ptr<WiiUPartition>> pPartitions);

    static std::optional<std::unique_ptr<WiiUPartition>> movePartitionFromList(std::vector<std::unique_ptr<WiiUPartition>> &list, std::string partitionName);
};