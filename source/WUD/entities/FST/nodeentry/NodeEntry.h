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

#include "NodeEntryParam.h"
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <WUD/entities/FST/sectionentry/SectionEntry.h>
#include <WUD/entities/FST/stringtable/StringEntry.h>
#include <cstdint>
#include <utils/blocksize/SectionBlockSize.h>
#include <utils/logger.h>

#define ENTRY_TYPE_File      0
#define ENTRY_TYPE_Directory 1
#define ENTRY_TYPE_Link      0x80

class DirectoryEntry;

class NodeEntry {

public:
    uint16_t permission{};
    std::shared_ptr<StringEntry> nameString;
    std::shared_ptr<SectionEntry> sectionEntry;
    std::optional<std::shared_ptr<DirectoryEntry>> parent;
    uint8_t entryType{};
    uint32_t entryNumber{};

    static std::optional<std::shared_ptr<NodeEntry>>
    AutoDeserialize(const std::vector<uint8_t> &data,
                    uint32_t offset,
                    const std::optional<std::shared_ptr<DirectoryEntry>> &pParent,
                    uint32_t eEntryNumber,
                    const std::shared_ptr<SectionEntries> &sectionEntries,
                    const std::shared_ptr<StringTable> &stringTable, const SectionBlockSize &blockSize);

    virtual ~NodeEntry() = default;

    virtual void printPathRecursive();

    [[nodiscard]] std::string getFullPath();

    [[nodiscard]] std::string getPath();

    [[nodiscard]] std::string getName();

    [[nodiscard]] bool isDirectory() const;

    [[nodiscard]] bool isFile() const;

    static constexpr uint32_t LENGTH = 16;

    [[nodiscard]] std::string getFullPathInternal();

protected:
    NodeEntry(uint16_t pPermission,
              std::shared_ptr<StringEntry> pNameString,
              std::shared_ptr<SectionEntry> pSectionEntry,
              std::optional<std::shared_ptr<DirectoryEntry>> pParent,
              uint8_t pType,
              uint32_t pEntryNumber);
};