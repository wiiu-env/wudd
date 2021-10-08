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

#include <cstdint>
#include <utils/blocksize/SectionBlockSize.h>
#include <WUD/entities/FST/stringtable/StringEntry.h>
#include <WUD/entities/FST/sectionentry/SectionEntry.h>
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <utils/logger.h>
#include "NodeEntryParam.h"

#define ENTRY_TYPE_File        0
#define ENTRY_TYPE_Directory   1
#define ENTRY_TYPE_Link        0x80

class DirectoryEntry;

class NodeEntry {

public:
    uint16_t permission{};
    StringEntry *nameString{};
    SectionEntry *sectionEntry{};
    DirectoryEntry *parent{};
    uint8_t entryType{};
    uint32_t entryNumber{};

    static NodeEntry *AutoDeserialize(uint8_t *data, uint32_t offset, DirectoryEntry *pParent, uint32_t eEntryNumber, SectionEntries *sectionEntries,
                                      StringTable *stringTable, const SectionBlockSize &blockSize);

    virtual ~NodeEntry() = default;

    virtual void printPathRecursive() {
        DEBUG_FUNCTION_LINE("%s", getFullPath().c_str());
    }

    [[nodiscard]] std::string getFullPath() const &;

    [[nodiscard]] std::string getPath() const &;

    [[nodiscard]] std::string getName() const &;

    [[nodiscard]] bool isDirectory() const;

    [[nodiscard]] bool isFile() const;

    static uint32_t LENGTH;
private:
    [[nodiscard]] std::string getFullPathInternal() const &;
};