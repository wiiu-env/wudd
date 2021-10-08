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

#include <WUD/entities/FST/stringtable/StringTable.h>
#include <utils/blocksize/SectionBlockSize.h>
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include "DirectoryEntry.h"
#include "RootEntry.h"
#include "NodeEntry.h"
#include "NodeEntries.h"

class NodeEntries {

public:
    explicit NodeEntries(RootEntry *pEntry);

    ~NodeEntries();

    static NodeEntry *
    DeserializeImpl(unsigned char *data, uint32_t offset, DirectoryEntry *parent, uint32_t entryNumber, SectionEntries *sectionEntries, StringTable *stringTable, const SectionBlockSize &blockSize);

    static NodeEntries *parseData(unsigned char *data, uint32_t offset, SectionEntries *sectionEntries, StringTable *stringTable, const SectionBlockSize &blockSize);

    RootEntry *rootEntry;
};
