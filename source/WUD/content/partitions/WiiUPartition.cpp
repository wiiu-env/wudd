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
#include <coreinit/debug.h>
#include "WiiUPartition.h"

uint32_t WiiUPartition::LENGTH = 128;

WiiUPartition::WiiUPartition() = default;

WiiUPartition::~WiiUPartition() {
    for (auto const&[key, val]: volumes) {
        delete val;
    }
    volumes.clear();
}

WiiUPartition::WiiUPartition(DiscReader *reader, uint32_t offset, const DiscBlockSize &blockSize) {
    auto buffer = (uint8_t *) malloc(LENGTH);
    if (buffer == nullptr) {
        OSFatal("WiiUPartition: alloc buffer failed");
    }
    if (!reader->hasDiscKey) {
        if (!reader->readEncrypted(buffer, offset, LENGTH)) {
            OSFatal("WiiUPartition: Failed to read encrypted");
        }
    } else {
        auto bufferBigger = (uint8_t *) malloc(LENGTH + 0x10);
        if (bufferBigger == nullptr) {
            OSFatal("WiiUPartition: alloc bufferBigger failed");
        }
        if (!reader->readDecrypted(bufferBigger, offset - 0x10, 0, LENGTH + 0x10, reader->discKey, nullptr, true)) {
            OSFatal("WiiUPartition: Failed to read encrypted");
        }

        memcpy(buffer, bufferBigger + 0x10, LENGTH);

        free(bufferBigger);
    }

    char name[32];
    memset(name, 0, sizeof(name));
    memcpy(name, buffer, 31);
    volumeId = name;
    uint8_t num = buffer[31];

    for (int i = 0; i < num; i++) {
        auto address = *((uint32_t *) &buffer[32 + (i * 4)]);
        AddressInDiscBlocks discLbaAddress = AddressInDiscBlocks(blockSize, address);
        auto vh = new VolumeHeader(reader, discLbaAddress.getAddressInBytes());
        volumes[discLbaAddress] = vh;
    }

    fileSystemDescriptor = ((uint16_t *) &buffer[64])[0];

    free(buffer);
}

uint64_t WiiUPartition::getSectionOffsetOnDefaultPartition() {
    if (volumes.size() != 1) {
        OSFatal("We have more or less than 1 volume header.");
    }
    return volumes.begin()->first.getAddressInBytes();
}


