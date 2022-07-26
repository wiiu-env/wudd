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

#include "VolumeHeader.h"
#include <coreinit/debug.h>
#include <memory>
#include <utils/logger.h>

uint32_t VolumeHeader::MAGIC = 0xCC93A4F5;

std::vector<std::unique_ptr<H3HashArray>> VolumeHeader::getH3HashArray(uint8_t *h3Data, uint32_t pNumberOfH3HashArray, uint32_t pH3HashArrayListSize) {
    std::vector<std::unique_ptr<H3HashArray>> arrayList;
    if (pNumberOfH3HashArray == 0) {
        return arrayList;
    }

    for (uint32_t i = 1; i < pNumberOfH3HashArray; i++) {
        auto *offsetPtr    = (uint32_t *) &h3Data[i * 4];
        uint32_t curOffset = offsetPtr[0];
        uint32_t curEnd    = pH3HashArrayListSize;
        if (i < pNumberOfH3HashArray - 1) {
            // If it's not the last element, the end of our .h3 is the start of the next .h3
            curEnd = offsetPtr[1];
        }

        arrayList.push_back(std::make_unique<H3HashArray>(h3Data + curOffset, curEnd - curOffset));
    }

    return arrayList;
}

std::optional<std::unique_ptr<VolumeHeader>> VolumeHeader::make_unique(std::shared_ptr<DiscReader> &discReader, uint64_t offset) {
    auto buffer = make_unique_nothrow<uint8_t[]>((size_t) 64);
    if (!buffer) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc buffer");
        return {};
    }

    if (!discReader->readEncrypted(buffer.get(), offset, 64)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read data");
        return {};
    }

    auto *bufferUint = (uint32_t *) buffer.get();

    if (bufferUint[0] != MAGIC) {
        DEBUG_FUNCTION_LINE_ERR("MAGIC mismatch");
        return {};
    }

    auto blockSize            = VolumeBlockSize(bufferUint[1]);
    auto volumeSize           = SizeInVolumeBlocks(blockSize, bufferUint[2]);
    auto h3HashArrayListSize  = bufferUint[3];
    auto numberOfH3HashArray  = bufferUint[4];
    auto FSTSize              = bufferUint[5];
    auto FSTAddress           = AddressInVolumeBlocks(blockSize, bufferUint[6]);
    auto FSTHashMode          = buffer[36];
    auto encryptType          = buffer[37];
    auto majorVersion         = buffer[38];
    auto minorVersion         = buffer[39];
    auto expiringMajorVersion = buffer[40];

    auto alignedH3ArrayListSize = ROUNDUP(h3HashArrayListSize, 16);
    auto bufferH3               = make_unique_nothrow<uint8_t[]>(alignedH3ArrayListSize);
    if (!bufferH3) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc h3 buffer");
        return {};
    }

    if (!discReader->readEncrypted(bufferH3.get(), offset + 64, alignedH3ArrayListSize)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to read h3 data");
        return {};
    }

    auto h3HashArrayList = getH3HashArray(bufferH3.get(), numberOfH3HashArray, h3HashArrayListSize);

    return std::unique_ptr<VolumeHeader>(
            new VolumeHeader(blockSize, volumeSize, FSTSize, FSTAddress, FSTHashMode, encryptType, majorVersion, minorVersion, expiringMajorVersion, std::move(h3HashArrayList), h3HashArrayListSize,
                             numberOfH3HashArray));
}

VolumeHeader::VolumeHeader(const VolumeBlockSize &pBlockSize,
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
                           uint32_t pNumberOfH3HashArray) : blockSize(pBlockSize),
                                                            volumeSize(std::move(pVolumeSize)),
                                                            FSTSize(pFSTSize),
                                                            FSTAddress(std::move(pFSTAddress)),
                                                            FSTHashMode(pFSTHashMode),
                                                            encryptType(pEncryptType),
                                                            majorVersion(pMajorVersion),
                                                            minorVersion(pMinorVersion),
                                                            expiringMajorVersion(pExpiringMajorVersion),
                                                            h3HashArrayList(std::move(pH3HashArrayList)),
                                                            h3HashArrayListSize(pH3HashArrayListSize),
                                                            numberOfH3HashArray(pNumberOfH3HashArray) {
}
