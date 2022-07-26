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

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <utils/blocksize/SectionBlockSize.h>

class FSTHeader {

public:
    static constexpr uint32_t LENGTH = 32;

    static std::optional<std::unique_ptr<FSTHeader>> make_unique(const std::array<uint8_t, FSTHeader::LENGTH> &data);

    uint8_t FSTVersion;
    SectionBlockSize blockSize;
    uint32_t numberOfSections;
    uint8_t hashDisabled;

private:
    FSTHeader(uint8_t pFSTVersion, SectionBlockSize pBlockSize, uint32_t pNumberOfSections, uint8_t pHashDisabled);
};
