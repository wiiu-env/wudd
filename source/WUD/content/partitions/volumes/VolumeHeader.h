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

#include "H3HashArray.h"
#include <WUD/DiscReader.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <utils/blocksize/AddressInVolumeBlocks.h>
#include <utils/blocksize/SizeInVolumeBlocks.h>
#include <utils/blocksize/VolumeBlockSize.h>
#include <utils/utils.h>
#include <vector>

class VolumeHeader {

public:
    static std::vector<std::unique_ptr<H3HashArray>> getH3HashArray(uint8_t *h3Data, uint32_t numberOfH3HashArray, uint32_t h3HashArrayListSize);

    static uint32_t MAGIC;
    VolumeBlockSize blockSize;
    SizeInVolumeBlocks volumeSize;
    uint32_t FSTSize;
    AddressInVolumeBlocks FSTAddress;
    uint8_t FSTHashMode;
    uint8_t encryptType;
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint8_t expiringMajorVersion;
    std::vector<std::unique_ptr<H3HashArray>> h3HashArrayList;

    uint32_t h3HashArrayListSize;
    uint32_t numberOfH3HashArray;

    static std::optional<std::unique_ptr<VolumeHeader>> make_unique(std::shared_ptr<DiscReader> &discReader, uint64_t offset);

private:
    VolumeHeader(
            const VolumeBlockSize &pBlockSize,
            SizeInVolumeBlocks pVolumeSize,
            uint32_t pFSTSize,
            AddressInVolumeBlocks pFSTAddress,
            uint8_t pFSTHashMode,
            uint8_t pEncryptType,
            uint8_t pMajorVersion,
            uint8_t pMinorVersion,
            uint8_t pExpiringMajorVersion,
            std::vector<std::unique_ptr<H3HashArray>> pH3HashArrayList,
            uint32_t pH3HashArrayListSize,
            uint32_t pNumberOfH3HashArray);
};