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


std::optional<std::shared_ptr<NodeEntry>>
FileEntry::parseData(const std::array<uint8_t, NodeEntry::LENGTH> &data,
                     const NodeEntryParam &param,
                     const std::shared_ptr<SectionEntries> &sectionEntries,
                     const std::shared_ptr<StringTable> &stringTable,
                     const SectionBlockSize &blockSize) {
    auto size   = ((uint32_t *) &data[8])[0];
    auto offset = SectionAddress(blockSize, ((uint32_t *) &data[4])[0]);

    auto stringNameOpt = stringTable->getStringEntry(param.uint24);
    if (!stringNameOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get string name");
        return {};
    }

    auto sectionEntryOpt = sectionEntries->getSection(param.sectionNumber);
    if (!sectionEntryOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get section entry");
        return {};
    }

    return std::shared_ptr<FileEntry>(new FileEntry(param, stringNameOpt.value(), sectionEntryOpt.value(), size, offset));
}

std::shared_ptr<SectionEntry> FileEntry::getSectionEntry() {
    return sectionEntry;
}

uint64_t FileEntry::getOffset() const {
    return address.getAddressInBytes();
}

uint32_t FileEntry::getSize() const {
    return size;
}

FileEntry::FileEntry(
        const NodeEntryParam &param,
        const std::shared_ptr<StringEntry> &pStringEntry,
        const std::shared_ptr<SectionEntry> &pSectionEntry,
        uint32_t pSize, SectionAddress pAddress) : NodeEntry(param.permission,
                                                             pStringEntry,
                                                             pSectionEntry,
                                                             param.parent,
                                                             param.type,
                                                             param.entryNumber),
                                                   address(std::move(pAddress)),
                                                   size(pSize) {
}