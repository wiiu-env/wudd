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

#include "DirectoryEntry.h"
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <WUD/entities/FST/stringtable/StringTable.h>
#include <cstdint>
#include <utils/blocksize/SectionBlockSize.h>

class RootEntry : public DirectoryEntry {

public:
    ~RootEntry() override = default;

    static std::optional<std::shared_ptr<NodeEntry>>
    parseData(const std::array<uint8_t, NodeEntry::LENGTH> &data,
              const NodeEntryParam &param,
              const std::shared_ptr<SectionEntries> &sectionEntries,
              const std::shared_ptr<StringTable> &stringTable);

    static uint32_t parseLastEntryNumber(const std::array<uint8_t, NodeEntry::LENGTH> &data);

private:
    explicit RootEntry(const std::shared_ptr<DirectoryEntry> &input);
};