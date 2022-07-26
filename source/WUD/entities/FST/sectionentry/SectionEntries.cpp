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
#include "SectionEntries.h"

std::optional<std::shared_ptr<SectionEntry>> SectionEntries::getSection(uint16_t sectionNumber) const {
    for (auto const &e : list) {
        if (e->sectionNumber == sectionNumber) {
            return e;
        }
    }
    return {};
}

uint32_t SectionEntries::getSizeInBytes() const {
    return list.size() * SectionEntry::LENGTH;
}

uint32_t SectionEntries::size() const {
    return list.size();
}

std::optional<std::shared_ptr<SectionEntries>> SectionEntries::make_shared(const std::vector<uint8_t> &data, uint32_t numberOfSections, const VolumeBlockSize &pBlockSize) {
    std::vector<std::shared_ptr<SectionEntry>> list;
    for (uint32_t i = 0; i < numberOfSections; i++) {
        if (data.size() < (i + 1) * SectionEntry::LENGTH) {
            DEBUG_FUNCTION_LINE_ERR("Failed to parse SectionEntries");
            return {};
        }
        std::array<uint8_t, SectionEntry::LENGTH> sectionEntryData{};
        memcpy(sectionEntryData.data(), data.data() + (i * SectionEntry::LENGTH), SectionEntry::LENGTH);
        list.push_back(std::make_shared<SectionEntry>(sectionEntryData, i, pBlockSize));
    }
    return std::shared_ptr<SectionEntries>(new SectionEntries(list));
}

std::vector<std::shared_ptr<SectionEntry>> SectionEntries::getSections() const & {
    return list;
}

SectionEntries::SectionEntries(std::vector<std::shared_ptr<SectionEntry>> pList) : list(std::move(pList)) {
}
