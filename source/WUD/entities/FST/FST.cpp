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
#include "FST.h"
#include <algorithm>

std::shared_ptr<RootEntry> FST::getRootEntry() const {
    return nodeEntries->getRootEntry();
}

std::optional<std::shared_ptr<FST>> FST::make_shared(const std::vector<uint8_t> &data, uint32_t offset, const VolumeBlockSize &blockSize) {
    uint32_t curOffset = offset;

    if (curOffset + FSTHeader::LENGTH > data.size()) {
        DEBUG_FUNCTION_LINE_ERR("Not enough data to parse the FSTHeader");
        return {};
    }

    std::array<uint8_t, FSTHeader::LENGTH> fstData{};
    std::copy_n(data.begin() + (int) curOffset, FSTHeader::LENGTH, fstData.begin());

    auto headerOpt = FSTHeader::make_unique(fstData);
    if (!headerOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse FSTHeader");
        return {};
    }
    curOffset += FSTHeader::LENGTH;

    uint32_t sectionEntriesDataSize = headerOpt.value()->numberOfSections * SectionEntry::LENGTH;
    if (curOffset + sectionEntriesDataSize > data.size()) {
        DEBUG_FUNCTION_LINE_ERR("Not enough data to parse the SectionEntries");
        return {};
    }

    std::vector<uint8_t> sectionEntriesData;
    sectionEntriesData.resize(sectionEntriesDataSize);
    std::copy_n(data.begin() + (int) curOffset, sectionEntriesDataSize, sectionEntriesData.begin());

    auto sectionEntriesOpt = SectionEntries::make_shared(sectionEntriesData, headerOpt.value()->numberOfSections, blockSize);
    if (!sectionEntriesOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse FSTHeader");
        return {};
    }
    curOffset += sectionEntriesOpt.value()->getSizeInBytes();

    std::array<uint8_t, NodeEntry::LENGTH> rootEntry{};
    std::copy_n(data.begin() + (int) curOffset, NodeEntry::LENGTH, rootEntry.begin());

    uint32_t lastEntryNumber = RootEntry::parseLastEntryNumber(rootEntry);

    auto stringTableOffset = curOffset + (lastEntryNumber * 16);

    auto stringTableOpt = StringTable::make_shared(data, stringTableOffset, lastEntryNumber);
    if (!stringTableOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse StringTable");
        return {};
    }

    auto nodeEntriesOpt = NodeEntries::make_unique(data, curOffset, sectionEntriesOpt.value(), stringTableOpt.value(), headerOpt.value()->blockSize);
    if (!nodeEntriesOpt.has_value()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to parse NodeEntries");
        return {};
    }

    return std::shared_ptr<FST>(new FST(
            std::move(headerOpt.value()),
            std::move(sectionEntriesOpt.value()),
            std::move(stringTableOpt.value()),
            std::move(nodeEntriesOpt.value())));
}

FST::FST(std::unique_ptr<FSTHeader> pHeader,
         std::shared_ptr<SectionEntries> pSectionEntries,
         std::shared_ptr<StringTable> pStringTable,
         std::unique_ptr<NodeEntries> pNodeEntries) : sectionEntries(std::move(pSectionEntries)),
                                                      stringTable(std::move(pStringTable)),
                                                      nodeEntries(std::move(pNodeEntries)),
                                                      header(std::move(pHeader)) {
}
