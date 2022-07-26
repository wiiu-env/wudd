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

#include "volumes/VolumeHeader.h"
#include <map>
#include <utility>
#include <utils/blocksize/AddressInDiscBlocks.h>
#include <utils/blocksize/DiscBlockSize.h>

class WiiUPartition {

public:
    virtual ~WiiUPartition();

    static constexpr uint32_t LENGTH = 128;

    virtual uint64_t getSectionOffsetOnDefaultPartition();

    [[nodiscard]] virtual const std::string &getVolumeId() const;

    [[nodiscard]] virtual const std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> &getVolumes() const;

    [[nodiscard]] virtual uint16_t getFileSystemDescriptor() const;

    static std::optional<std::unique_ptr<WiiUPartition>> make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset, const DiscBlockSize &blockSize);

protected:
    WiiUPartition() = default;

private:
    WiiUPartition(std::string pVolumeId, std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> pVolumes, uint16_t pFileSystemDescriptor)
        : volumeId(std::move(pVolumeId)), volumes(std::move(pVolumes)), fileSystemDescriptor(pFileSystemDescriptor) {
    }

    const std::string volumeId;
    std::map<AddressInDiscBlocks, std::unique_ptr<VolumeHeader>> volumes;
    uint16_t fileSystemDescriptor{};
};