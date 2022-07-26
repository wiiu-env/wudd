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

#include <utility>
#include <utils/blocksize/SectionAddress.h>

#include "NodeEntry.h"

class FileEntry : public NodeEntry {
public:
    static std::optional<std::shared_ptr<NodeEntry>>
    parseData(const std::array<uint8_t, NodeEntry::LENGTH> &data,
              const NodeEntryParam &param,
              const std::shared_ptr<SectionEntries> &sectionEntries,
              const std::shared_ptr<StringTable> &stringTable,
              const SectionBlockSize &blockSize);

    ~FileEntry() override = default;

    std::shared_ptr<SectionEntry> getSectionEntry();

    [[nodiscard]] uint64_t getOffset() const;

    [[nodiscard]] uint32_t getSize() const;

private:
    FileEntry(const NodeEntryParam &param,
              const std::shared_ptr<StringEntry> &pStringEntry,
              const std::shared_ptr<SectionEntry> &pSectionEntry,
              uint32_t pSize,
              SectionAddress offset);

    SectionAddress address;
    uint32_t size{};
};