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

#include <WUD/DiscReader.h>
#include <memory>
#include <optional>
#include <string>
#include <utility>

class WiiUDiscId {

public:
    static constexpr uint32_t LENGTH = 32768;
    static constexpr uint32_t MAGIC  = 0xCC549EB9;
    uint8_t minorVersion;
    uint8_t majorVersion;
    std::string footprint;

    static std::optional<std::unique_ptr<WiiUDiscId>> make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset);

private:
    WiiUDiscId(uint8_t pMinorVersion, uint8_t pMajorVersion, std::string pFootprint);
};
