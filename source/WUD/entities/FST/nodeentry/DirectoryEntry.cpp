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
#include "DirectoryEntry.h"

DirectoryEntry *DirectoryEntry::parseData(const uint8_t *data, NodeEntryParam param, SectionEntries *sectionEntries, StringTable *stringTable) {
    auto *directoryEntry = new DirectoryEntry();
    directoryEntry->entryNumber = param.entryNumber;
    directoryEntry->parent = param.parent;
    directoryEntry->entryType = param.type;
    directoryEntry->nameString = stringTable->getStringEntry(param.uint24);
    if (directoryEntry->nameString == nullptr) {
        OSFatal("Failed to find string for offset");
    }

    directoryEntry->parentEntryNumber = ((uint32_t *) &data[4])[0];
    directoryEntry->lastEntryNumber = ((uint32_t *) &data[8])[0];

    directoryEntry->permission = param.permission;

    if (param.sectionNumber > sectionEntries->size()) {
        OSFatal("section number does not match");
    }
    directoryEntry->sectionEntry = sectionEntries->getSection(param.sectionNumber);

    return directoryEntry;
}

DirectoryEntry::~DirectoryEntry() {
    for (auto &child: children) {
        delete child;
    }
}

std::vector<DirectoryEntry *> DirectoryEntry::getDirChildren() const {
    std::vector<DirectoryEntry *> res;
    for (auto &cur: children) {
        if (cur->isDirectory()) {
            res.push_back(dynamic_cast<DirectoryEntry *>(cur));
        }
    }
    return res;
}

std::vector<FileEntry *> DirectoryEntry::getFileChildren() const {
    std::vector<FileEntry *> res;
    for (auto &cur: children) {
        if (cur->isFile()) {
            res.push_back(dynamic_cast<FileEntry *>(cur));
        }
    }
    return res;
}

std::vector<NodeEntry *> DirectoryEntry::getChildren() const {
    return children;
}

void DirectoryEntry::printPathRecursive() {
    DEBUG_FUNCTION_LINE("%s", getFullPath().c_str());
    for (auto &child: children) {
        child->printPathRecursive();
    }
}

void DirectoryEntry::addChild(NodeEntry *entry) {
    children.push_back(entry);
}
