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
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <utils/blocksize/DiscBlockSize.h>

class WiiUDiscContentsHeader {

public:
    DiscBlockSize blockSize{};
    uint32_t numberOfPartition;
    std::array<uint8_t, 20> tocHash;

    static constexpr uint32_t LENGTH = 2048;
    static constexpr uint32_t MAGIC  = 0xCCA6E67B;

    static std::optional<std::unique_ptr<WiiUDiscContentsHeader>> make_unique(std::shared_ptr<DiscReader> &discReader, uint32_t offset);

private:
    WiiUDiscContentsHeader(DiscBlockSize pSize, const std::array<uint8_t, 20> &pTocHash, uint32_t pNumberOfPartitions);
};
