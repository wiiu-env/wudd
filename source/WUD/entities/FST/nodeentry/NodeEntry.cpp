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
#include <utils/blocksize/SectionBlockSize.h>
#include <WUD/entities/FST/stringtable/StringTable.h>
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <coreinit/debug.h>
#include "NodeEntry.h"
#include "DirectoryEntry.h"
#include "RootEntry.h"

uint32_t  NodeEntry::LENGTH = 16;

NodeEntry *NodeEntry::AutoDeserialize(uint8_t *data, uint32_t offset, DirectoryEntry *pParent, uint32_t eEntryNumber, SectionEntries *sectionEntries,
                                      StringTable *stringTable, const SectionBlockSize &blockSize) {
    uint8_t *curEntryData = &data[offset];

    NodeEntryParam param{};
    param.permission = ((uint16_t *) &curEntryData[12])[0];
    param.sectionNumber = ((uint16_t *) &curEntryData[14])[0];
    param.entryNumber = eEntryNumber;
    param.parent = pParent;
    param.type = curEntryData[0];
    param.uint24 = ((uint32_t *) curEntryData)[0] & 0x00FFFFFF;

    if ((param.type & ENTRY_TYPE_Directory) == ENTRY_TYPE_Directory && param.uint24 == 0) { // Root
        return (NodeEntry *) RootEntry::parseData(curEntryData, param, sectionEntries, stringTable);
    } else if ((param.type & ENTRY_TYPE_Directory) == ENTRY_TYPE_Directory) {
        return (NodeEntry *) DirectoryEntry::parseData(curEntryData, param, sectionEntries, stringTable);
    } else if ((param.type & ENTRY_TYPE_File) == ENTRY_TYPE_File) {
        return (NodeEntry *) FileEntry::parseData(curEntryData, param, sectionEntries, stringTable, blockSize);
    }

    OSFatal("FST Unknown Node Type");
    return nullptr;
}

std::string NodeEntry::getName() const &{
    if (nameString != nullptr) {
        return nameString->toString();
    }
    return "ERROR";
}

std::string NodeEntry::getFullPathInternal() const &{
    if (parent != nullptr) {
        return parent->getFullPathInternal().append("/").append(getName());
    }
    return getName();
}

std::string NodeEntry::getFullPath() const &{
    return getFullPathInternal();
}

std::string NodeEntry::getPath() const &{
    if (parent != nullptr) {
        return parent->getFullPath().append("/");
    }
    return "/";
}

bool NodeEntry::isDirectory() const {
    return (entryType & ENTRY_TYPE_Directory) == ENTRY_TYPE_Directory;
}


bool NodeEntry::isFile() const {
    return (entryType & ENTRY_TYPE_File) == ENTRY_TYPE_File;
}