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

#include <map>
#include <utils/blocksize/DiscBlockSize.h>
#include <utils/blocksize/AddressInDiscBlocks.h>
#include "volumes/VolumeHeader.h"

class WiiUPartition {

public:
    WiiUPartition();
    explicit WiiUPartition(DiscReader *reader, uint32_t offset, const DiscBlockSize &blockSize);

    virtual uint64_t getSectionOffsetOnDefaultPartition();

    virtual ~WiiUPartition();

    [[nodiscard]] virtual std::string getVolumeId() const &{
        return volumeId;
    }

    [[nodiscard]] virtual std::map<AddressInDiscBlocks, VolumeHeader *> getVolumes() const&{
        return volumes;
    }

    [[nodiscard]] virtual uint16_t getFileSystemDescriptor() const{
        return fileSystemDescriptor;
    }

private:
    std::string volumeId;
    std::map<AddressInDiscBlocks, VolumeHeader *> volumes;
    uint16_t fileSystemDescriptor{};
    static uint32_t LENGTH;
};