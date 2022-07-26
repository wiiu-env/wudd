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
#include "RootEntry.h"
#include <coreinit/debug.h>

RootEntry::RootEntry(const std::shared_ptr<DirectoryEntry> &input) : DirectoryEntry(input) {
}

uint32_t RootEntry::parseLastEntryNumber(const std::array<uint8_t, NodeEntry::LENGTH> &data) {
    return ((uint32_t *) &data[8])[0];
}

std::optional<std::shared_ptr<NodeEntry>>
RootEntry::parseData(const std::array<uint8_t, NodeEntry::LENGTH> &data,
                     const NodeEntryParam &param,
                     const std::shared_ptr<SectionEntries> &sectionEntries,
                     const std::shared_ptr<StringTable> &stringTable) {
    auto dir = DirectoryEntry::parseData(data, param, sectionEntries, stringTable);
    if (dir.has_value()) {
        if ((dir.value()->entryType & ENTRY_TYPE_Directory) != ENTRY_TYPE_Directory || dir.value()->entryNumber != 0) {
            DEBUG_FUNCTION_LINE_ERR("Input is no root entry.");
            return {};
        }
        return std::shared_ptr<NodeEntry>(new RootEntry(dir.value()));
    }
    DEBUG_FUNCTION_LINE_ERR("Failed to parse dir");
    return {};
}
