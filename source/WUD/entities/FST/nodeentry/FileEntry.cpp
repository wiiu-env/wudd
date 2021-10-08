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
#include "FileEntry.h"
#include <WUD/entities/FST/stringtable/StringTable.h>
#include <coreinit/debug.h>

#include <utility>

FileEntry::FileEntry(SectionAddress address) : address(std::move(address)) {

}

NodeEntry *FileEntry::parseData(uint8_t *data, NodeEntryParam param, SectionEntries *sectionEntries, StringTable *stringTable, const SectionBlockSize &blockSize) {
    auto *entry = new FileEntry(SectionAddress(blockSize, ((uint32_t *) &data[4])[0]));

    entry->entryNumber = param.entryNumber;
    entry->parent = param.parent;
    entry->entryType = param.type;
    entry->nameString = stringTable->getStringEntry(param.uint24);
    if (entry->nameString == nullptr) {
        OSFatal("Failed to find string for offset");
    }

    entry->size = ((uint32_t *) &data[8])[0];

    entry->permission = param.permission;
    entry->sectionEntry = sectionEntries->getSection(param.sectionNumber);

    return entry;
}

SectionEntry *FileEntry::getSectionEntry() {
    return sectionEntry;
}

uint64_t FileEntry::getOffset() const {
    return address.getAddressInBytes();
}

uint32_t FileEntry::getSize() const {
    return size;
}
