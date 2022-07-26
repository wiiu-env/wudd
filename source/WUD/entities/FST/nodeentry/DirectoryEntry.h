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

#include "FileEntry.h"
#include "NodeEntry.h"
#include "NodeEntryParam.h"
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <WUD/entities/FST/stringtable/StringTable.h>

class DirectoryEntry : public NodeEntry {
public:
    static std::optional<std::shared_ptr<DirectoryEntry>>
    parseData(const std::array<uint8_t, NodeEntry::LENGTH> &data,
              const NodeEntryParam &param,
              const std::shared_ptr<SectionEntries> &sectionEntries,
              const std::shared_ptr<StringTable> &stringTable);

    uint32_t parentEntryNumber{};
    uint32_t lastEntryNumber{};
    std::vector<std::shared_ptr<NodeEntry>> children;

    void addChild(const std::shared_ptr<NodeEntry> &entry);

    [[nodiscard]] std::vector<std::shared_ptr<DirectoryEntry>> getDirChildren() const;

    [[nodiscard]] std::vector<std::shared_ptr<FileEntry>> getFileChildren() const;

    [[nodiscard]] std::vector<std::shared_ptr<NodeEntry>> getChildren() const;

    void printPathRecursive() override;

protected:
    explicit DirectoryEntry(const std::shared_ptr<DirectoryEntry> &input);

private:
    DirectoryEntry(
            const NodeEntryParam &param,
            const std::shared_ptr<StringEntry> &stringEntry,
            const std::shared_ptr<SectionEntry> &sectionEntry,
            uint32_t parentEntryNumber,
            uint32_t lastEntryNumber);
};