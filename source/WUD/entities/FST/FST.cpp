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
#include "FST.h"

FST::FST(uint8_t *data, uint32_t fstSize, uint32_t offset, const VolumeBlockSize &blockSize) {
    uint32_t curOffset = offset;

    header = new FSTHeader(data + curOffset);
    curOffset += header->LENGTH;

    sectionEntries = new SectionEntries(data + curOffset, header->numberOfSections, blockSize);
    curOffset += sectionEntries->getSizeInBytes();

    uint32_t lastEntryNumber = RootEntry::parseLastEntryNumber(data, curOffset);

    auto stringTableOffset = curOffset + (lastEntryNumber * 16);

    stringTable = StringTable::parseData(data, fstSize - stringTableOffset, stringTableOffset, lastEntryNumber);

    nodeEntries = NodeEntries::parseData(data, curOffset, sectionEntries, stringTable, header->blockSize);
}

FST::~FST() {
    delete nodeEntries;
    delete stringTable;
    delete sectionEntries;
    delete header;
}

RootEntry *FST::getRootEntry() const {
    return nodeEntries->rootEntry;
}

