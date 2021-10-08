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
#include "NodeEntryParam.h"
#include "NodeEntry.h"
#include "FileEntry.h"

class DirectoryEntry : public NodeEntry {


public:
    ~DirectoryEntry() override;

    static DirectoryEntry *parseData(const uint8_t *data, NodeEntryParam param, SectionEntries *sectionEntries, StringTable *stringTable);

    uint32_t parentEntryNumber{};
    uint32_t lastEntryNumber{};
    std::vector<NodeEntry *> children;

    void addChild(NodeEntry *entry);

    [[nodiscard]] std::vector<DirectoryEntry *> getDirChildren() const;

    [[nodiscard]] std::vector<FileEntry *> getFileChildren() const;

    [[nodiscard]] std::vector<NodeEntry *> getChildren() const;

    void printPathRecursive() override;
};