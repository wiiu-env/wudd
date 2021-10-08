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

#include <vector>
#include <cstdint>
#include <WUD/DiscReader.h>
#include <utils/blocksize/VolumeBlockSize.h>
#include <utils/blocksize/AddressInVolumeBlocks.h>
#include <utils/blocksize/SizeInVolumeBlocks.h>
#include "H3HashArray.h"

class VolumeHeader {

public:
    static std::vector<H3HashArray *> getH3HashArray(uint8_t *h3Data, uint32_t numberOfH3HashArray, uint32_t h3HashArrayListSize);

    VolumeHeader(DiscReader *reader, uint64_t offset);

    ~VolumeHeader();

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
    std::vector<H3HashArray *> h3HashArrayList;

    uint32_t h3HashArrayListSize;
    uint32_t numberOfH3HashArray;
};