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
#include <coreinit/debug.h>
#include "RootEntry.h"

RootEntry::RootEntry(DirectoryEntry *input) {
    if ((input->entryType & ENTRY_TYPE_Directory) != ENTRY_TYPE_Directory || input->entryNumber != 0) {
        OSFatal("Input is no root entry.");
    }

    entryNumber = input->entryNumber;
    parent = input->parent;
    nameString = input->nameString;
    if(nameString == nullptr){
        OSFatal("nameString was null");
    }
    entryType = input->entryType;

    parentEntryNumber = input->parentEntryNumber;
    lastEntryNumber = input->lastEntryNumber;
    permission = input->permission;
    sectionEntry = input->sectionEntry;
}

uint32_t RootEntry::parseLastEntryNumber(uint8_t *data, uint32_t offset) {
    return ((uint32_t *) &data[8 + offset])[0];
}

RootEntry *RootEntry::parseData(uint8_t *data, NodeEntryParam param, SectionEntries *sectionEntries, StringTable *stringTable) {
    return new RootEntry(DirectoryEntry::parseData(data, param, sectionEntries, stringTable));
}
