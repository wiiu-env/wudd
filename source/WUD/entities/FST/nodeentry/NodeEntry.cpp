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
#include <WUD/entities/FST/sectionentry/SectionEntries.h>
#include <WUD/entities/FST/stringtable/StringTable.h>
#include <utils/blocksize/SectionBlockSize.h>

#include "DirectoryEntry.h"
#include "NodeEntry.h"
#include "RootEntry.h"
#include <algorithm>
#include <utility>

std::optional<std::shared_ptr<NodeEntry>>
NodeEntry::AutoDeserialize(const std::vector<uint8_t> &data,
                           uint32_t offset,
                           const std::optional<std::shared_ptr<DirectoryEntry>> &pParent,
                           uint32_t eEntryNumber,
                           const std::shared_ptr<SectionEntries> &sectionEntries,
                           const std::shared_ptr<StringTable> &stringTable,
                           const SectionBlockSize &blockSize) {
    if (offset + NodeEntry::LENGTH >= data.size()) {
        return {};
    }
    std::array<uint8_t, NodeEntry::LENGTH> curEntryData{};
    std::copy_n(data.begin() + (int) offset, NodeEntry::LENGTH, curEntryData.begin());

    NodeEntryParam param{};
    param.permission    = ((uint16_t *) &curEntryData[12])[0];
    param.sectionNumber = ((uint16_t *) &curEntryData[14])[0];
    param.entryNumber   = eEntryNumber;
    param.parent        = pParent;
    param.type          = curEntryData[0];
    param.uint24        = ((uint32_t *) &curEntryData[0])[0] & 0x00FFFFFF;

    if ((param.type & ENTRY_TYPE_Directory) == ENTRY_TYPE_Directory && param.uint24 == 0) { // Root
        auto res = RootEntry::parseData(curEntryData, param, sectionEntries, stringTable);
        if (!res.has_value()) {
            DEBUG_FUNCTION_LINE_ERR("Failed to parse node");
            return {};
        }
        return res;
    } else if ((param.type & ENTRY_TYPE_Directory) == ENTRY_TYPE_Directory) {
        auto res = DirectoryEntry::parseData(curEntryData, param, sectionEntries, stringTable);
        if (!res.has_value()) {
            DEBUG_FUNCTION_LINE_ERR("Failed to parse node");
            return {};
        }
        auto resAsNodeEntry = std::dynamic_pointer_cast<NodeEntry>(res.value());
        if (resAsNodeEntry == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("Failed to cast to NodeEntry");
            return {};
        }
        return resAsNodeEntry;
    } else if ((param.type & ENTRY_TYPE_File) == ENTRY_TYPE_File) {
        auto res = FileEntry::parseData(curEntryData, param, sectionEntries, stringTable, blockSize);

        if (!res.has_value()) {
            DEBUG_FUNCTION_LINE_ERR("Failed to parse node");
            return {};
        }
        auto resAsNodeEntry = std::dynamic_pointer_cast<NodeEntry>(res.value());
        if (resAsNodeEntry == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("Failed to cast to NodeEntry");
            return {};
        }
        return resAsNodeEntry;
    }

    DEBUG_FUNCTION_LINE_ERR("FST Unknown Node Type");
    return {};
}

std::string NodeEntry::getName() {
    auto res = nameString->toString();
    if (res.has_value()) {
        return res.value();
    }
    return "[ERROR]";
}

std::string NodeEntry::getFullPathInternal() {
    if (parent.has_value()) {
        return parent.value()->getFullPathInternal().append("/").append(getName());
    }
    return getName();
}

std::string NodeEntry::getFullPath() {
    return getFullPathInternal();
}

std::string NodeEntry::getPath() {
    if (parent.has_value()) {
        return parent.value()->getFullPath().append("/");
    }
    return "/";
}

bool NodeEntry::isDirectory() const {
    return (entryType & ENTRY_TYPE_Directory) == ENTRY_TYPE_Directory;
}


bool NodeEntry::isFile() const {
    return (entryType & ENTRY_TYPE_File) == ENTRY_TYPE_File;
}

NodeEntry::NodeEntry(const uint16_t pPermission,
                     std::shared_ptr<StringEntry> pNameString,
                     std::shared_ptr<SectionEntry> pSectionEntry,
                     std::optional<std::shared_ptr<DirectoryEntry>> pParent,
                     const uint8_t pType, const uint32_t pEntryNumber) : permission(pPermission),
                                                                         nameString(std::move(pNameString)),
                                                                         sectionEntry(std::move(pSectionEntry)),
                                                                         parent(std::move(pParent)),
                                                                         entryType(pType),
                                                                         entryNumber(pEntryNumber) {
}

void NodeEntry::printPathRecursive() {
    DEBUG_FUNCTION_LINE("%s", getFullPath().c_str());
}
