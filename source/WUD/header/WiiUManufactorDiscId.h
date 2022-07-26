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
#include <cstdint>
#include <memory>
#include <optional>

class WiiUManufactorDiscId {

public:
    static std::optional<std::unique_ptr<WiiUManufactorDiscId>> make_unique(const std::shared_ptr<DiscReader> &discReader);

    static constexpr uint32_t LENGTH = 65536;

    std::array<uint8_t, WiiUManufactorDiscId::LENGTH> data;

private:
    explicit WiiUManufactorDiscId(const std::array<uint8_t, WiiUManufactorDiscId::LENGTH> &pData);
};
