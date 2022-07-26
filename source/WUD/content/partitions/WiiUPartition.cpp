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

std::optional<std::unique_ptr<WiiUPartition>> WiiUPartition::make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset, const DiscBlockSize &blockSize) {
    // If we have the discKey, the content is encrypted but we don't know the IV.
    // So in this case we read the 0x10 bytes before to the actual offset get the IV.
    auto bufferReal = make_unique_nothrow<uint8_t[]>(discReader->hasDiscKey ? LENGTH + 0x10 : LENGTH);
    if (!bufferReal) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate memory");
        return {};
    }
    uint8_t *buffer = bufferReal.get();
    if (!discReader->hasDiscKey) {
        if (!discReader->readEncrypted(bufferReal.get(), offset, LENGTH)) {
            return {};
        }
    } else {
        if (offset < 0x10) {
            DEBUG_FUNCTION_LINE_ERR("Tried to read from an invalid offset");
            OSFatal("Tried to read from an invalid offset");
        }
        if (!discReader->readDecrypted(bufferReal.get(), offset - 0x10, 0, LENGTH + 0x10, discReader->discKey, nullptr, true)) {
            return {};
        }
        buffer = bufferReal.get() + 0x10;
    }

    char name[32];
    memset(name, 0, sizeof(name));
    memcpy(name, buffer, 31);
    std::string volumeId = name;
    uint8_t num          = buffer[31];

    std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> volumes;

    for (int i = 0; i < num; i++) {
        auto address                       = *((uint32_t *) &buffer[32 + (i * 4)]);
        AddressInDiscBlocks discLbaAddress = AddressInDiscBlocks(blockSize, address);
        auto vh                            = VolumeHeader::make_unique(discReader, discLbaAddress.getAddressInBytes());
        if (!vh.has_value()) {
            return {};
        }
        volumes[discLbaAddress] = std::move(vh.value());
    }

    auto fileSystemDescriptor = ((uint16_t *) &buffer[64])[0];

    return std::unique_ptr<WiiUPartition>(new WiiUPartition(
            volumeId,
            std::move(volumes),
            fileSystemDescriptor));
}

const std::string &WiiUPartition::getVolumeId() const {
    return volumeId;
}

const std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> &WiiUPartition::getVolumes() const {
    return volumes;
}

uint16_t WiiUPartition::getFileSystemDescriptor() const {
    return fileSystemDescriptor;
}

WiiUPartition::~WiiUPartition() = default;
