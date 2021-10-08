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
#include <utils/utils.h>
#include <coreinit/debug.h>

uint32_t  VolumeHeader::MAGIC = 0xCC93A4F5;

VolumeHeader::VolumeHeader(DiscReader *reader, uint64_t offset) {
    auto buffer = (uint8_t *) malloc(64);
    if (buffer == nullptr) {
        OSFatal("VolumeHeader: failed to alloc buffer");
    }

    if (!reader->readEncrypted(buffer, offset, 64)) {
        OSFatal("VolumeHeader: failed to read");
    }

    auto *bufferUint = (uint32_t *) buffer;

    if (bufferUint[0] != MAGIC) {
        OSFatal("VolumeHeader MAGIC mismatch.");
    }

    blockSize = VolumeBlockSize(bufferUint[1]);
    volumeSize = SizeInVolumeBlocks(blockSize, bufferUint[2]);
    h3HashArrayListSize = bufferUint[3];
    numberOfH3HashArray = bufferUint[4];
    FSTSize = bufferUint[5];
    FSTAddress = AddressInVolumeBlocks(blockSize, bufferUint[6]);
    FSTHashMode = buffer[36];
    encryptType = buffer[37];
    majorVersion = buffer[38];
    minorVersion = buffer[39];
    expiringMajorVersion = buffer[40];

    free(buffer);

    auto bufferH3 = (uint8_t *) malloc(ROUNDUP(h3HashArrayListSize, 16));
    if (bufferH3 == nullptr) {
        OSFatal("VolumeHeader: failed to alloc h3 buffer");
    }

    if (!reader->readEncrypted(bufferH3, offset + 64, ROUNDUP(h3HashArrayListSize, 16))) {
        OSFatal("VolumeHeader: failed to read h3");
    }

    h3HashArrayList = getH3HashArray(bufferH3, numberOfH3HashArray, h3HashArrayListSize);

    free(bufferH3);
}

std::vector<H3HashArray *> VolumeHeader::getH3HashArray(uint8_t *h3Data, uint32_t pNumberOfH3HashArray, uint32_t pH3HashArrayListSize) {
    std::vector<H3HashArray *> arrayList;
    if (pNumberOfH3HashArray == 0) {
        return arrayList;
    }

    for (uint32_t i = 1; i < pNumberOfH3HashArray; i++) {
        auto *offsetPtr = (uint32_t *) &h3Data[i * 4];
        uint32_t curOffset = offsetPtr[0];
        uint32_t curEnd = pH3HashArrayListSize;
        if (i < pNumberOfH3HashArray - 1) {
            // If it's not the last element, the end of our .h3 is the start of the next .h3
            curEnd = offsetPtr[1];
        }

        arrayList.push_back(new H3HashArray(h3Data + curOffset, curEnd - curOffset));
    }

    return arrayList;
}

VolumeHeader::~VolumeHeader() {
    for (auto &h3: h3HashArrayList) {
        delete h3;
    }
}

