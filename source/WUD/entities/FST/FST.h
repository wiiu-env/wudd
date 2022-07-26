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

#include <WUD/entities/FST/header/FSTHeader.h>
#include <WUD/entities/FST/nodeentry/NodeEntries.h>
#include <WUD/entities/FST/nodeentry/RootEntry.h>
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <WUD/entities/FST/stringtable/StringTable.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

class FST {
public:
    [[nodiscard]] std::shared_ptr<RootEntry> getRootEntry() const;

    static std::optional<std::shared_ptr<FST>> make_shared(const std::vector<uint8_t> &data, uint32_t offset, const VolumeBlockSize &blockSize);

    std::shared_ptr<SectionEntries> sectionEntries;

private:
    FST(std::unique_ptr<FSTHeader> pHeader,
        std::shared_ptr<SectionEntries> pSectionEntries,
        std::shared_ptr<StringTable> pStringTable,
        std::unique_ptr<NodeEntries> pNodeEntries);

    std::shared_ptr<StringTable> stringTable;
    std::unique_ptr<NodeEntries> nodeEntries;
    std::unique_ptr<FSTHeader> header;
};