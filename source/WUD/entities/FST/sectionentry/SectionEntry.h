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

#include <array>
#include <cstdint>
#include <string>
#include <utils/blocksize/AddressInVolumeBlocks.h>
#include <utils/blocksize/SizeInVolumeBlocks.h>

class SectionEntry {
public:
    static constexpr uint32_t LENGTH = 32;

    SectionEntry(const std::array<uint8_t, SectionEntry::LENGTH> &data, uint32_t pSectionNumber, const VolumeBlockSize &pBlockSize);

    ~SectionEntry() = default;

    AddressInVolumeBlocks address;
    SizeInVolumeBlocks size;
    uint8_t hashMode;
    uint64_t ownerID;
    uint64_t groupID;
    std::string name;
    uint32_t sectionNumber;
};
