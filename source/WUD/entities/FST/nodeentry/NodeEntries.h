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

#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <WUD/entities/FST/stringtable/StringTable.h>
#include <utils/blocksize/SectionBlockSize.h>

#include "DirectoryEntry.h"
#include "NodeEntries.h"
#include "NodeEntry.h"
#include "RootEntry.h"
#include <utility>

class NodeEntries {

public:
    static std::optional<std::shared_ptr<NodeEntry>>
    DeserializeImpl(const std::vector<uint8_t> &data,
                    uint32_t offset,
                    const std::optional<std::shared_ptr<DirectoryEntry>> &pParent,
                    uint32_t entryNumber,
                    const std::shared_ptr<SectionEntries> &sectionEntries,
                    const std::shared_ptr<StringTable> &stringTable,
                    const SectionBlockSize &blockSize);

    static std::optional<std::unique_ptr<NodeEntries>>
    make_unique(const std::vector<uint8_t> &data,
                uint32_t offset,
                const std::shared_ptr<SectionEntries> &sectionEntries,
                const std::shared_ptr<StringTable> &stringTable,
                const SectionBlockSize &blockSize);

    [[nodiscard]] std::shared_ptr<RootEntry> getRootEntry() const;

private:
    explicit NodeEntries(const std::shared_ptr<RootEntry> &pEntry);


    std::shared_ptr<RootEntry> rootEntry;
};
