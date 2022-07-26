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

#include "SectionEntry.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <utils/blocksize/VolumeBlockSize.h>
#include <utils/logger.h>
#include <vector>

class SectionEntries {

public:
    [[nodiscard]] uint32_t getSizeInBytes() const;

    [[nodiscard]] uint32_t size() const;

    [[nodiscard]] std::optional<std::shared_ptr<SectionEntry>> getSection(uint16_t sectionNumber) const;

    [[nodiscard]] std::vector<std::shared_ptr<SectionEntry>> getSections() const &;

    static std::optional<std::shared_ptr<SectionEntries>> make_shared(const std::vector<uint8_t> &data, uint32_t numberOfSections, const VolumeBlockSize &pBlockSize);

private:
    explicit SectionEntries(std::vector<std::shared_ptr<SectionEntry>> pList);

    std::vector<std::shared_ptr<SectionEntry>> list;
};