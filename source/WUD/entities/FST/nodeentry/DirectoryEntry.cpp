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
#include "DirectoryEntry.h"
#include <coreinit/debug.h>

std::optional<std::shared_ptr<DirectoryEntry>>
DirectoryEntry::parseData(const std::array<uint8_t, NodeEntry::LENGTH> &data, const NodeEntryParam &param, const std::shared_ptr<SectionEntries> &sectionEntries,
                          const std::shared_ptr<StringTable> &stringTable) {
    auto parentEntryNumber = ((uint32_t *) &data[4])[0];
    auto lastEntryNumber   = ((uint32_t *) &data[8])[0];
    auto stringNameOpt     = stringTable->getStringEntry(param.uint24);
    if (!stringNameOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get string name");
        return {};
    }

    auto sectionEntryOpt = sectionEntries->getSection(param.sectionNumber);
    if (!sectionEntryOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get section entry");
        return {};
    }


    return std::unique_ptr<DirectoryEntry>(new DirectoryEntry(param, stringNameOpt.value(), sectionEntryOpt.value(), parentEntryNumber, lastEntryNumber));
}

std::vector<std::shared_ptr<DirectoryEntry>> DirectoryEntry::getDirChildren() const {
    std::vector<std::shared_ptr<DirectoryEntry>> res;
    for (auto &cur : children) {
        if (cur->isDirectory()) {
            res.push_back(std::dynamic_pointer_cast<DirectoryEntry>(cur));
        }
    }
    return res;
}

std::vector<std::shared_ptr<FileEntry>> DirectoryEntry::getFileChildren() const {
    std::vector<std::shared_ptr<FileEntry>> res;
    for (auto &cur : children) {
        if (cur->isFile()) {
            res.push_back(std::dynamic_pointer_cast<FileEntry>(cur));
        }
    }
    return res;
}

std::vector<std::shared_ptr<NodeEntry>> DirectoryEntry::getChildren() const {
    return children;
}

void DirectoryEntry::printPathRecursive() {
    DEBUG_FUNCTION_LINE("%s", getFullPath().c_str());
    for (auto &child : children) {
        child->printPathRecursive();
    }
}

void DirectoryEntry::addChild(const std::shared_ptr<NodeEntry> &entry) {
    children.push_back(entry);
}

DirectoryEntry::DirectoryEntry(const NodeEntryParam &param,
                               const std::shared_ptr<StringEntry> &pStringEntry,
                               const std::shared_ptr<SectionEntry> &pSectionEntry,
                               uint32_t pParentEntryNumber,
                               uint32_t pLastEntryNumber) : NodeEntry(param.permission,
                                                                      pStringEntry,
                                                                      pSectionEntry,
                                                                      param.parent,
                                                                      param.type,
                                                                      param.entryNumber),
                                                            parentEntryNumber(pParentEntryNumber),
                                                            lastEntryNumber(pLastEntryNumber) {
}

DirectoryEntry::DirectoryEntry(const std::shared_ptr<DirectoryEntry> &input) : NodeEntry(input->permission,
                                                                                         input->nameString,
                                                                                         input->sectionEntry,
                                                                                         input->parent,
                                                                                         input->entryType,
                                                                                         input->entryNumber),
                                                                               parentEntryNumber(input->parentEntryNumber),
                                                                               lastEntryNumber(input->lastEntryNumber) {
}
