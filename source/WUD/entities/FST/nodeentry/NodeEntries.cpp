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
#include "NodeEntries.h"

NodeEntries::NodeEntries(RootEntry *pEntry) {
    rootEntry = pEntry;
}

NodeEntries::~NodeEntries() {
    delete rootEntry;
}

NodeEntry *NodeEntries::DeserializeImpl(uint8_t *data, uint32_t offset, DirectoryEntry *parent, uint32_t entryNumber, SectionEntries *sectionEntries, StringTable *stringTable,
                                        const SectionBlockSize &blockSize) {
    NodeEntry *nodeEntry = NodeEntry::AutoDeserialize(data, offset, parent, entryNumber, sectionEntries, stringTable, blockSize);
    auto asDirEntry = dynamic_cast<DirectoryEntry *>(nodeEntry);
    if (asDirEntry != nullptr) {
        uint32_t curEntryNumber = asDirEntry->entryNumber + 1;
        while (curEntryNumber < asDirEntry->lastEntryNumber) {
            NodeEntry *entry = NodeEntries::DeserializeImpl(data, offset + (curEntryNumber - asDirEntry->entryNumber) * NodeEntry::LENGTH,
                                                            asDirEntry, curEntryNumber, sectionEntries, stringTable, blockSize);
            asDirEntry->addChild(entry);
            auto *childAsDir = dynamic_cast<DirectoryEntry *>(entry);
            if (childAsDir != nullptr) {
                curEntryNumber = childAsDir->lastEntryNumber;
            } else {
                curEntryNumber++;
            }
        }
    }
    return nodeEntry;
}

NodeEntries *NodeEntries::parseData(unsigned char *data, uint32_t offset, SectionEntries *sectionEntries, StringTable *stringTable, const SectionBlockSize &blockSize) {
    NodeEntry *rootEntry = NodeEntries::DeserializeImpl(data, offset, (DirectoryEntry *) nullptr, 0, sectionEntries, stringTable, blockSize);
    auto rootEntryCasted = dynamic_cast<RootEntry *>(rootEntry);
    if (rootEntryCasted != nullptr) {
        return new NodeEntries(rootEntryCasted);
    }
    OSFatal("Failed to parse Root");
    return nullptr;
}
