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
#include "NodeEntries.h"
#include <coreinit/debug.h>

std::optional<std::shared_ptr<NodeEntry>> NodeEntries::DeserializeImpl(const std::vector<uint8_t> &pData,
                                                                       uint32_t pOffset,
                                                                       const std::optional<std::shared_ptr<DirectoryEntry>> &pParent,
                                                                       uint32_t pEntryNumber,
                                                                       const std::shared_ptr<SectionEntries> &pSectionEntries,
                                                                       const std::shared_ptr<StringTable> &pStringTable,
                                                                       const SectionBlockSize &pBlockSize) {
    auto nodeEntry = NodeEntry::AutoDeserialize(pData, pOffset, pParent, pEntryNumber, pSectionEntries, pStringTable, pBlockSize);
    if (!nodeEntry.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to AutoDeserialize NodeEntry");
        return {};
    }
    auto asDirEntry = std::dynamic_pointer_cast<DirectoryEntry>(nodeEntry.value());
    if (asDirEntry != nullptr) {
        uint32_t curEntryNumber = asDirEntry->entryNumber + 1;
        while (curEntryNumber < asDirEntry->lastEntryNumber) {
            auto entry = NodeEntries::DeserializeImpl(pData, pOffset + (curEntryNumber - asDirEntry->entryNumber) * NodeEntry::LENGTH,
                                                      asDirEntry, curEntryNumber, pSectionEntries, pStringTable, pBlockSize);
            if (!entry.has_value()) {
                DEBUG_FUNCTION_LINE_ERR("Failed to Deserialize child of NodeEntry");
                return {};
            }
            asDirEntry->addChild(entry.value());
            auto childAsDir = std::dynamic_pointer_cast<DirectoryEntry>(entry.value());
            if (childAsDir != nullptr) {
                curEntryNumber = childAsDir->lastEntryNumber;
            } else {
                curEntryNumber++;
            }
        }
    }
    return nodeEntry;
}

std::optional<std::unique_ptr<NodeEntries>>
NodeEntries::make_unique(const std::vector<uint8_t> &data, uint32_t offset, const std::shared_ptr<SectionEntries> &pSectionEntries, const std::shared_ptr<StringTable> &pStringTable,
                         const SectionBlockSize &blockSize) {
    auto rootEntry = NodeEntries::DeserializeImpl(data, offset, std::nullopt, 0, pSectionEntries, pStringTable, blockSize);
    if (!rootEntry.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("DeserializeImpl for root entry has failed");
        return {};
    }
    auto rootEntryCasted = std::dynamic_pointer_cast<RootEntry>(rootEntry.value());
    if (rootEntryCasted != nullptr) {
        return std::unique_ptr<NodeEntries>(new NodeEntries(rootEntryCasted));
    }
    DEBUG_FUNCTION_LINE_ERR("Failed to parse Root");
    return {};
}

std::shared_ptr<RootEntry> NodeEntries::getRootEntry() const {
    return rootEntry;
}

NodeEntries::NodeEntries(const std::shared_ptr<RootEntry> &pEntry) {
    rootEntry = pEntry;
}
