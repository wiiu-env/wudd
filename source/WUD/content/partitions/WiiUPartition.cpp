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
#include "WiiUPartition.h"
#include <coreinit/debug.h>
#include <memory>

uint64_t WiiUPartition::getSectionOffsetOnDefaultPartition() {
    if (volumes.size() != 1) {
        OSFatal("We have more or less than 1 volume header.");
    }
    return volumes.begin()->first.getAddressInBytes();
}

std::optional<std::shared_ptr<WiiUPartition>> WiiUPartition::make_shared(const std::shared_ptr<DiscReader> &discReader, uint32_t offset, const DiscBlockSize &blockSize) {
    auto buffer = (uint8_t *) malloc(LENGTH);
    if (buffer == nullptr) {
        return {};
    }
    if (!discReader->hasDiscKey) {
        if (!discReader->readEncrypted(buffer, offset, LENGTH)) {
            return {};
        }
    } else {
        auto bufferBigger = (uint8_t *) malloc(LENGTH + 0x10);
        if (bufferBigger == nullptr) {
            return {};
        }
        if (!discReader->readDecrypted(bufferBigger, offset - 0x10, 0, LENGTH + 0x10, discReader->discKey, nullptr, true)) {
            return {};
        }

        memcpy(buffer, bufferBigger + 0x10, LENGTH);

        free(bufferBigger);
    }

    char name[32];
    memset(name, 0, sizeof(name));
    memcpy(name, buffer, 31);
    auto volumeId = name;
    uint8_t num   = buffer[31];

    std::map<AddressInDiscBlocks, std::shared_ptr<VolumeHeader>> volumes;

    for (int i = 0; i < num; i++) {
        auto address                       = *((uint32_t *) &buffer[32 + (i * 4)]);
        AddressInDiscBlocks discLbaAddress = AddressInDiscBlocks(blockSize, address);
        auto vh                            = VolumeHeader::make_shared(discReader, discLbaAddress.getAddressInBytes());
        if (!vh.has_value()) {
            free(buffer);
            return {};
        }
        volumes[discLbaAddress] = vh.value();
    }

    auto fileSystemDescriptor = ((uint16_t *) &buffer[64])[0];

    free(buffer);

    return std::unique_ptr<WiiUPartition>(new WiiUPartition(
            volumeId,
            volumes,
            fileSystemDescriptor));
}

std::string WiiUPartition::getVolumeId() const & {
    return volumeId;
}

std::map<AddressInDiscBlocks, std::shared_ptr<VolumeHeader>> WiiUPartition::getVolumes() const & {
    return volumes;
}

uint16_t WiiUPartition::getFileSystemDescriptor() const {
    return fileSystemDescriptor;
}

WiiUPartition::~WiiUPartition() = default;
