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
#include "SectionEntry.h"

SectionEntry::SectionEntry(const std::array<uint8_t, SectionEntry::LENGTH> &data, uint32_t pSectionNumber, const VolumeBlockSize &pBlockSize) {
    auto *dataAsUint = (uint32_t *) data.data();
    address          = AddressInVolumeBlocks(pBlockSize, dataAsUint[0]);
    size             = SizeInVolumeBlocks(pBlockSize, dataAsUint[1]);

    ownerID  = ((uint64_t *) (&data[8]))[0];
    groupID  = ((uint64_t *) (&data[16]))[0];
    hashMode = data[20];
    char buff[32];
    snprintf(buff, sizeof(buff), "Section: %d", pSectionNumber);

    name          = std::string(buff);
    sectionNumber = pSectionNumber;
}
